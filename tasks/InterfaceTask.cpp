/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include "InterfaceTask.hpp"

using namespace canbus;

InterfaceTask::InterfaceTask(std::string const& name, TaskCore::TaskState initial_state)
    : InterfaceTaskBase(name, initial_state)
{
}

InterfaceTask::InterfaceTask(std::string const& name, RTT::ExecutionEngine* engine, TaskCore::TaskState initial_state)
    : InterfaceTaskBase(name, engine, initial_state)
{
}

InterfaceTask::~InterfaceTask()
{
}

bool InterfaceTask::configureHook()
{
    if (! RTT::TaskContext::configureHook())
        return false;
    return true;
}

bool InterfaceTask::startHook()
{
    if (! RTT::TaskContext::startHook())
        return false;
    return true;
}

void InterfaceTask::updateHook()
{
    RTT::TaskContext::updateHook();
}

void InterfaceTask::errorHook()
{
    RTT::TaskContext::errorHook();
}

void InterfaceTask::stopHook()
{
    RTT::TaskContext::stopHook();
}

void InterfaceTask::cleanupHook()
{
    
    RTT::TaskContext::cleanupHook();
}

bool InterfaceTask::readCanMsg(canbus::Message& msg){
    return _can_in.read(msg) == RTT::NewData;
}

bool InterfaceTask::sendCanMsg(const canbus::Message &msg){
    _can_out.write(msg);
    return true;
}

