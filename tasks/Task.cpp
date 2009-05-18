#include "Task.hpp"

#include <rtt/FileDescriptorActivity.hpp>


using namespace can;


RTT::FileDescriptorActivity* Task::getFileDescriptorActivity()
{ return dynamic_cast< RTT::FileDescriptorActivity* >(getActivity().get()); }


Task::Task(std::string const& name)
    : TaskBase(name)
{
    // we don't want any waiting
    m_driver.setReadTimeout(0);
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

int Task::watch(std::string name, int id, int mask)
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
    if (!m_driver.open(_device.get()))
        return false;

    getFileDescriptorActivity()->watch(m_driver.getFileDescriptor());
    return true;
}
bool Task::startHook()
{
    return m_driver.reset();
}

void Task::updateHook(std::vector<RTT::PortInterface*> const& updated_ports)
{
    can::Message msg;

    // Write the data that is available on the input ports
    typedef std::vector<RTT::PortInterface*> UpdatedPorts;
    for (UpdatedPorts::const_iterator it = updated_ports.begin(); it != updated_ports.end(); ++it)
    {
        RTT::InputPort<can::Message>* port = dynamic_cast<RTT::InputPort<can::Message> *> (*it);
        while (port->read(msg))
            m_driver.write(msg);
    }

    // Read the data on the file descriptor (if there is any) and push it on the
    // matching port
    try { 
        while(true)
        {
            msg = m_driver.read();
            for (Mappings::const_iterator it = m_mappings.begin(); it != m_mappings.end(); ++it)
            {
                if ((msg.can_id & it->mask) == it->id)
                    it->output->write(msg);
            }
        }
    }
    catch(timeout_error) { }
}

void Task::stopHook()
{
    m_driver.close();
}

