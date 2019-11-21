/* Generated from orogen/lib/orogen/templates/tasks/Task.hpp */

#ifndef CANBUS_TASK_TASK_HPP
#define CANBUS_TASK_TASK_HPP

#include "canbus/TaskBase.hpp"
#include <rtt/OutputPort.hpp>

namespace canbus {
    class Driver;

    class Task : public TaskBase
    {
        friend class TaskBase;
    protected:

        /* Handler for the unwatch operation
         */
        virtual bool watch(std::string const& name, int id, int mask);
        /* Handler for the watch operation
         */
        virtual bool unwatch(std::string const& name);

        struct Mapping
        {
            std::string name;
            uint32_t id;
            uint32_t mask;
            RTT::OutputPort<canbus::Message>* output;
        };
        typedef std::vector<Mapping> Mappings;

        struct MappingCacheItem
        {
            int id;
            std::vector<RTT::OutputPort<canbus::Message>* > outputs;
        };
        typedef std::map<uint32_t, MappingCacheItem> MappingCache;

        canbus::Driver *m_driver;
        canbus::Statistics m_stats;
        Mappings    m_mappings;
        MappingCache m_mapping_cache;
        base::Time m_last_can_check_time;
        base::Time m_last_stats_time;

        base::Time m_can_check_interval;
        base::Time m_stats_interval;

    public:
        Task(std::string const& name = "canbus::Task");
        ~Task();

        /** This hook is called by Orocos when the state machine transitions
         * from PreOperational to Stopped. If it returns false, then the
         * component will stay in PreOperational. Otherwise, it goes into
         * Stopped.
         *
         * It is meaningful only if the #needs_configuration has been specified
         * in the task context definition with (for example):
         *
         *   task_context "TaskName" do
         *     needs_configuration
         *     ...
         *   end
         */
        bool configureHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to Running. If it returns false, then the component will
         * stay in Stopped. Otherwise, it goes into Running and updateHook()
         * will be called.
         */
        bool startHook();

        /** This hook is called by Orocos when the component is in the Running
         * state, at each activity step. Here, the activity gives the "ticks"
         * when the hook should be called.
         *
         * The error(), exception() and fatal() calls, when called in this hook,
         * allow to get into the associated RunTimeError, Exception and
         * FatalError states.
         *
         * In the first case, updateHook() is still called, and recover() allows
         * you to go back into the Running state.  In the second case, the
         * errorHook() will be called instead of updateHook(). In Exception, the
         * component is stopped and recover() needs to be called before starting
         * it again. Finally, FatalError cannot be recovered.
         */
        void updateHook();


        /** This hook is called by Orocos when the component is in the
         * RunTimeError state, at each activity step. See the discussion in
         * updateHook() about triggering options.
         *
         * Call recovered() to go back in the Runtime state.
         */
        // void errorHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Running to Stopped after stop() has been called.
         */
        void stopHook();

        /** This hook is called by Orocos when the state machine transitions
         * from Stopped to PreOperational, requiring the call to configureHook()
         * before calling start() again.
         */
        void cleanupHook();

    };
}

#endif

