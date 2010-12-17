/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "Task.hpp"

#include <rtt/extras/FileDescriptorActivity.hpp>
#include <canbus.hh>
#include <iostream>

using namespace canbus;


Task::Task(std::string const& name)
    : TaskBase(name)
    , m_driver(NULL)
{
    //check every 100 packets if bus is ok
    _checkBusOkCount = 100;

    updateHookCallCount = 0;
}

Task::~Task()
{
    // delete all ports
    for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
    {
        ports()->removePort(it->output->getName());
        delete it->output;
    }
}

bool Task::watch(std::string const& name, int id, int mask)
{
    if (isRunning())
        return false;

    // Check if there is no port named like this already
    for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
    {
        if (it->name == name)
            return false;
    }

    // Create ports for both directions
    RTT::OutputPort<canbus::Message>* output_port = new RTT::OutputPort<canbus::Message>(name);
    ports()->addPort(name, *output_port);

    // And register the mapping
    Mapping mapping = { name, id & mask, mask, output_port };
    m_mappings.push_back(mapping);

    return true;
}

bool Task::unwatch(std::string const& name)
{
    // delete all ports
    for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
    {
        if (it->output->getName() == name)
        {
            ports()->removePort(it->output->getName());
            delete it->output;
            return true;
        }
    }
    return false;
}


bool Task::configureHook()
{
#if CANBUS_VERSION >= 101
    if (!(m_driver = canbus::openCanDevice(_device.get())))
    {
	std::cerr << "CANBUS: Failed to open device" << std::endl;
	return false;
    }
#else
    m_driver = new canbus::Driver();
    if (!m_driver->open(_device.get()))
    {
	std::cerr << "CANBUS: Failed to open device" << std::endl;
        return false;
    }
#endif

    // we don't want any waiting
    m_driver->setReadTimeout(0);

    return true;
}
bool Task::startHook()
{
    RTT::extras::FileDescriptorActivity* fd_activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
        fd_activity->watch(m_driver->getFileDescriptor());

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
            for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
            {
                if ((msg.can_id & it->mask) == it->id)
                {
                    cache_it = m_mapping_cache.insert( std::make_pair(msg.can_id, it->output) ).first;
                    break;
                }
            }
            if (cache_it == m_mapping_cache.end())
                cache_it = m_mapping_cache.insert( std::make_pair(msg.can_id, static_cast<RTT::OutputPort<canbus::Message>*>(0)) ).first;
        }

        if (cache_it->second)
            cache_it->second->write(msg);
    }

    updateHookCallCount++;
    if(updateHookCallCount > _checkBusOkCount) {
        updateHookCallCount = 0;
        if(!m_driver->checkBusOk()) {
	    std::cerr << "canbus reported error" << std::endl;
	    exception(IO_ERROR);
        }
    }

    m_stats.time = base::Time::now();
    _stats.write(m_stats);
}

void Task::stopHook()
{
    RTT::extras::FileDescriptorActivity* fd_activity =
        getActivity<RTT::extras::FileDescriptorActivity>();
    if (fd_activity)
        fd_activity->clearAllWatches();

    m_driver->close();
    delete m_driver;
    m_driver = 0;
}

