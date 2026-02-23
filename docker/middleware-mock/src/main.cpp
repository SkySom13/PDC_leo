#include <iostream>
#include <thread>
#include <chrono>

#include <CommonAPI/CommonAPI.hpp>
#include "core/v1/vehiclecontrol/VehicleControlStubDefault.hpp"
#include "someip/v1/vehiclecontrol/VehicleControlSomeIPDeployment.hpp"
#include "someip/v1/vehiclecontrol/VehicleControlSomeIPStubAdapter.hpp"

using namespace v1::vehiclecontrol;

class VehicleControlMockStub
    : public VehicleControlStubDefault
{
public:
    VehicleControlMockStub()
        : speed_(0)
    {}

    // Example: override a getter if your FIDL has one
    virtual void getCurrentSpeed(GetCurrentSpeedReply_t _reply) override {
        _reply(speed_);
    }

    void tick() {
        speed_ = (speed_ + 5) % 140;
        // If your FIDL has a field and event, update and fire it here
        // this->fireCurrentSpeedChanged(speed_);
    }

private:
    uint32_t speed_;
};

int main() {
    std::cout << "VehicleControlMockServer starting..." << std::endl;

    auto runtime = CommonAPI::Runtime::get();
    std::string domain = "local";
    std::string instance = "VehicleControl"; // adapt to your FIDL instance name
    std::string connection = "vehicle_mock"; // same as VSOMEIP_APPLICATION_NAME

    auto mockStub = std::make_shared<VehicleControlMockStub>();

    auto service = runtime->registerService<VehicleControlStub>(domain, instance, mockStub, connection);
    if (!service) {
        std::cerr << "Failed to register VehicleControl service" << std::endl;
        return 1;
    }

    std::cout << "VehicleControl service registered on CommonAPI domain '" << domain
              << "', instance '" << instance << "'" << std::endl;

    // Periodically update dummy state
    while (true) {
        mockStub->tick();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
