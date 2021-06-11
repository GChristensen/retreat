export module SchedulerFactory;

import <memory>;

import Settings;
import Scheduler;
import PeriodicScheduler;

export class SchedulerFactory {
public:
    static std::shared_ptr<Scheduler> createScheduler(Settings &settings);
};

module :private;

//std::shared_ptr<Scheduler> SchedulerFactory::createScheduler(const Settings *settings) {
//    return std::shared_ptr<PeriodicScheduler>(new PeriodicScheduler(settings));
//}


std::shared_ptr<Scheduler> SchedulerFactory::createScheduler(Settings &settings) {
    std::shared_ptr<PeriodicScheduler> scheduler(new PeriodicScheduler(settings));
    return scheduler;
}