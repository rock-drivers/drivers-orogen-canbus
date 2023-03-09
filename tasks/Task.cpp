/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

#include <rtt/extras/FileDescriptorActivity.hpp>
#include <canbus.hh>
#include <iostream>
#include <rtt/Logger.hpp>

using namespace canbus;

Task::Task(std::string const& name)
: TaskBase(name)
, m_driver(NULL)
{
}

Task::~Task()
{
    // delete all ports
    for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
    {
        ports()->removePort(it->output->getName());
        delete it->output;
    }
    delete m_driver;
}

bool Task::watch(std::string const& name, int id, int mask)
{
    if (isRunning())
        return false;

    if (provides()->hasService(name))
    {
        RTT::log(RTT::Error) << "cannot watch the specified CAN IDs on port " << name << " as the name is already in use in the task interface" << RTT::endlog();
        return false;
    }

    // Create ports for both directions
    RTT::OutputPort<canbus::Message>* output_port = new RTT::OutputPort<canbus::Message>(name);
    ports()->addPort(name, *output_port);

    // And register the mapping
    Mapping mapping = { name, id & mask, mask, false, output_port };
    m_mappings.push_back(mapping);

    return true;
}

bool Task::unwatch(std::string const& name)
{
    // delete all ports
    for (Mappings::iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
    {
        if (it->output->getName() == name)
        {
            m_mapping_cache.clear();
            ports()->removePort(it->output->getName());
            delete it->output;
            m_mappings.erase(it);
            return true;
        }
    }
    return false;
}


bool Task::configureHook()
{
    if (!(m_driver = canbus::openCanDevice(_device.get(), _deviceType.get())))
    {
        std::cerr << "CANBUS: Failed to open device" << std::endl;
        return false;
    }

    // creating dynamic ports
    outputports = _outputPorts.get();
    for (size_t i = 0; i < outputports.size(); ++i)
    {
        canbus::CanOutputPort const& outputport(outputports[i]);
        if (ports()->getPort(outputport.ports_name))
        {
            RTT::log(RTT::Error) << "output port " <<  outputport.ports_name << " is listed more than once in the outputs configuration property" << RTT::endlog();
            return false;
        }
        RTT::OutputPort<canbus::Message>* new_output_port = new RTT::OutputPort<canbus::Message>(outputport.ports_name);
        ports()->addPort(outputport.ports_name, *new_output_port);
        // register the mapping
        Mapping mapping = { outputport.ports_name, outputport.id & outputport.mask, outputport.mask, true, new_output_port };
        m_mappings.push_back(mapping);
    }
   
    m_can_check_interval = base::Time::fromMicroseconds(_checkBusOkInterval.get() * 1000);
    m_stats_interval = base::Time::fromMicroseconds(_statsInterval.get() * 1000);
    return true;
}

bool Task::startHook()
{
    RTT::extras::FileDescriptorActivity* fd_activity =
            getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
    {
        fd_activity->watch(m_driver->getFileDescriptor());
        fd_activity->setTimeout(100);
    }

    if (!m_driver->reset())
    {
        std::cerr << "CANBUS: Failed to reset can driver" << std::endl;
        return false;
    }
    m_driver->clear();
    m_mapping_cache.clear();
    return true;
}

void Task::updateHook()
{
    canbus::Message msg;

    // Write the data that is available on the input ports
    while (_in.read(msg) == RTT::NewData) {
        m_stats.msg_tx++;
        // CAN extended frames are 8 bytes of header, max 8 bytes of payload
        m_stats.tx += 8 + msg.size;
        m_driver->write(msg);
    }
    // Read the data on the file descriptor (if there is any) and push it on the
    // matching port. We ask the board how many packets there is to read.
    int msg_count = m_driver->getPendingMessagesCount();
    for (int i = 0; i < msg_count; ++i)
    {
        msg = m_driver->read();

        m_stats.msg_rx++;
        // CAN extended frames are 8 bytes of header, max 8 bytes of payload
        m_stats.rx += 8 + msg.size;

        MappingCache::const_iterator cache_it = m_mapping_cache.find(msg.can_id);
        if (cache_it == m_mapping_cache.end())
        {
            MappingCacheItem new_cache_item;
            new_cache_item.id = msg.can_id;

            for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); it++)
            {
                if ((msg.can_id & it->mask) == static_cast<uint32_t>(it->id))
                {
                    new_cache_item.outputs.push_back(it->output);
                }
            }

            cache_it = m_mapping_cache.insert(std::make_pair(new_cache_item.id, new_cache_item)).first;
        }

        for(std::vector<RTT::OutputPort<canbus::Message>*>::const_iterator it = cache_it->second.outputs.begin(); it != cache_it->second.outputs.end(); it++)
        {
            (*it)->write(msg);
        }
    }

    base::Time cur_time = base::Time::now();

    if(cur_time - m_last_can_check_time > m_can_check_interval)
    {
        m_last_can_check_time = cur_time;
        if(!m_driver->checkBusOk()) {
            std::cerr << "canbus reported error" << std::endl;
            exception(IO_ERROR);
        }
    }

    if(cur_time - m_last_stats_time > m_stats_interval)
    {
        m_stats.time = cur_time;
        m_stats.error_count =  m_driver->getErrorCount();
        m_last_stats_time = cur_time;
        _stats.write(m_stats);
    }
}

void Task::stopHook()
{
    RTT::extras::FileDescriptorActivity* fd_activity =
            getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
        fd_activity->clearAllWatches();

    TaskBase::stopHook();
}

void Task::cleanupHook()
{
    m_driver->close();
    delete m_driver;
    m_driver = 0;
    for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end();)
    {
        if (it->remove_on_cleanup)
        {
            m_mapping_cache.clear();
            ports()->removePort(it->output->getName());
            delete it->output;
            it = m_mappings.erase(it);
        }
        else 
            ++it;
    }
    TaskBase::cleanupHook();
}
