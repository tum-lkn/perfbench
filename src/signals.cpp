#include "signals.h"

boost::signals2::signal<void(Tenant* tenant)> Signals::TenantConnected;
boost::signals2::signal<void(Tenant* tenant)> Signals::TenantFailed;
boost::signals2::signal<void()> Signals::AllTenantsConnected;
boost::signals2::signal<void()> Signals::AllTenantsFinished;
