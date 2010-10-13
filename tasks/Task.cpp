#include "Task.hpp"

#include <rtt/FileDescriptorActivity.hpp>
#include <iostream>

using namespace can;


RTT::FileDescriptorActivity* Task::getFileDescriptorActivity()
{ return dynamic_cast< RTT::FileDescriptorActivity* >(getActivity().get()); }


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
        ports()->removePort(it->input->getName());

        delete it->input;
        delete it->output;
    }
}

int Task::watch(std::string const& name, int id, int mask)
{
    if (isActive())
        return false;

    // Check if there is no port named like this already
    for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
    {
        if (it->name == name)
            return false;
    }

    // Create ports for both directions
    RTT::OutputPort<can::Message>* output_port = new RTT::OutputPort<can::Message>(name);
    ports()->addPort(output_port);
    RTT::InputPort<can::Message>* input_port = new RTT::InputPort<can::Message>("w" + name);
    ports()->addEventPort(input_port);

    // And register the mapping
    Mapping mapping = { name, id & mask, mask, output_port, input_port };
    m_mappings.push_back(mapping);
    return true;
}


bool Task::configureHook()
{
#if CANBUS_VERSION >= 101
    if (!(m_driver = can::openCanDevice(_device.get())))
        return false;
#else
    if (!(m_driver = new can::Driver()))
        return false;
    if (!m_driver->open(_device.get()))
        return false;
#endif

    // we don't want any waiting
    m_driver->setReadTimeout(0);

    RTT::FileDescriptorActivity* fd_activity = getFileDescriptorActivity();
    if (fd_activity)
        fd_activity->watch(m_driver->getFileDescriptor());
    return true;
}
bool Task::startHook()
{
    if (!m_driver->reset())
        return false;
    m_driver->clear();
    return true;
}

void Task::updateHook()
{
    can::Message msg;

    // Write the data that is available on the input ports
    for (Mappings::iterator port_it = m_mappings.begin(); port_it != m_mappings.end(); ++port_it)
    {
        RTT::InputPort<can::Message>* port = port_it->input;
        while (port->read(msg)) {
            m_driver->write(msg);
	}
    }

    // Read the data on the file descriptor (if there is any) and push it on the
    // matching port. We ask the board how many packets there is to read.
    int msg_count = m_driver->getPendingMessagesCount();
    for (int i = 0; i < msg_count; ++i)
    {
        msg = m_driver->read();
        
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
                cache_it = m_mapping_cache.insert( std::make_pair(msg.can_id, static_cast<RTT::OutputPort<can::Message>*>(0)) ).first;
        }

        if (cache_it->second)
            cache_it->second->write(msg);
    }

    updateHookCallCount++;
    if(updateHookCallCount > _checkBusOkCount) {
        updateHookCallCount = 0;
        if(!m_driver->checkBusOk()) {
	    std::cerr << "canbus reported error" << std::endl;
	    fatal();
        }
    }
}

void Task::stopHook()
{
    m_driver->close();
    can::Driver *d = m_driver;
    m_driver = NULL;
    delete d;
}

