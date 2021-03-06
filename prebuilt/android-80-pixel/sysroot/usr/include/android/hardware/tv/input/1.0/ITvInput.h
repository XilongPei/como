#ifndef HIDL_GENERATED_ANDROID_HARDWARE_TV_INPUT_V1_0_ITVINPUT_H
#define HIDL_GENERATED_ANDROID_HARDWARE_TV_INPUT_V1_0_ITVINPUT_H

#include <android/hardware/tv/input/1.0/ITvInputCallback.h>
#include <android/hardware/tv/input/1.0/types.h>
#include <android/hidl/base/1.0/IBase.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace tv {
namespace input {
namespace V1_0 {

struct ITvInput : public ::android::hidl::base::V1_0::IBase {
    virtual bool isRemote() const override { return false; }


    // @entry @exit @callflow(next="getStreamConfigurations")
    virtual ::android::hardware::Return<void> setCallback(const ::android::sp<ITvInputCallback>& callback) = 0;

    using getStreamConfigurations_cb = std::function<void(Result result, const ::android::hardware::hidl_vec<TvStreamConfig>& configurations)>;
    // @callflow(next={"openStream", "getStreamConfigurations", "closeStream"})
    virtual ::android::hardware::Return<void> getStreamConfigurations(int32_t deviceId, getStreamConfigurations_cb _hidl_cb) = 0;

    using openStream_cb = std::function<void(Result result, const ::android::hardware::hidl_handle& sidebandStream)>;
    // @callflow(next={"closeStream", "getStreamConfigurations", "openStream"})
    virtual ::android::hardware::Return<void> openStream(int32_t deviceId, int32_t streamId, openStream_cb _hidl_cb) = 0;

    // @callflow(next={"getStreamConfigurations", "openStream", "closeStream"})
    virtual ::android::hardware::Return<Result> closeStream(int32_t deviceId, int32_t streamId) = 0;

    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;

    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> setHALInstrumentation() override;

    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;

    virtual ::android::hardware::Return<void> ping() override;

    using getDebugInfo_cb = std::function<void(const ::android::hidl::base::V1_0::DebugInfo& info)>;
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> notifySyspropsChanged() override;

    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;
    // cast static functions
    static ::android::hardware::Return<::android::sp<ITvInput>> castFrom(const ::android::sp<ITvInput>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<ITvInput>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<ITvInput> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<ITvInput> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<ITvInput> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<ITvInput> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<ITvInput> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<ITvInput> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<ITvInput> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<ITvInput> getService(bool getStub) { return getService("default", getStub); }
    ::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

std::string toString(const ::android::sp<ITvInput>&);

}  // namespace V1_0
}  // namespace input
}  // namespace tv
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_TV_INPUT_V1_0_ITVINPUT_H
