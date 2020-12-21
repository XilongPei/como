#ifndef HIDL_GENERATED_ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_BPHWGRAPHICBUFFERPRODUCER_H
#define HIDL_GENERATED_ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_BPHWGRAPHICBUFFERPRODUCER_H

#include <hidl/HidlTransportSupport.h>

#include <android/hardware/graphics/bufferqueue/1.0/IHwGraphicBufferProducer.h>

namespace android {
namespace hardware {
namespace graphics {
namespace bufferqueue {
namespace V1_0 {

struct BpHwGraphicBufferProducer : public ::android::hardware::BpInterface<IGraphicBufferProducer>, public ::android::hardware::details::HidlInstrumentor {
    explicit BpHwGraphicBufferProducer(const ::android::sp<::android::hardware::IBinder> &_hidl_impl);

    virtual bool isRemote() const override { return true; }

    // Methods from IGraphicBufferProducer follow.
    ::android::hardware::Return<void> requestBuffer(int32_t slot, requestBuffer_cb _hidl_cb) override;
    ::android::hardware::Return<int32_t> setMaxDequeuedBufferCount(int32_t maxDequeuedBuffers) override;
    ::android::hardware::Return<int32_t> setAsyncMode(bool async) override;
    ::android::hardware::Return<void> dequeueBuffer(uint32_t width, uint32_t height, ::android::hardware::graphics::common::V1_0::PixelFormat format, uint32_t usage, bool getFrameTimestamps, dequeueBuffer_cb _hidl_cb) override;
    ::android::hardware::Return<int32_t> detachBuffer(int32_t slot) override;
    ::android::hardware::Return<void> detachNextBuffer(detachNextBuffer_cb _hidl_cb) override;
    ::android::hardware::Return<void> attachBuffer(const ::android::hardware::media::V1_0::AnwBuffer& buffer, attachBuffer_cb _hidl_cb) override;
    ::android::hardware::Return<void> queueBuffer(int32_t slot, const IGraphicBufferProducer::QueueBufferInput& input, queueBuffer_cb _hidl_cb) override;
    ::android::hardware::Return<int32_t> cancelBuffer(int32_t slot, const ::android::hardware::hidl_handle& fence) override;
    ::android::hardware::Return<void> query(int32_t what, query_cb _hidl_cb) override;
    ::android::hardware::Return<void> connect(const ::android::sp<IProducerListener>& listener, int32_t api, bool producerControlledByApp, connect_cb _hidl_cb) override;
    ::android::hardware::Return<int32_t> disconnect(int32_t api, IGraphicBufferProducer::DisconnectMode mode) override;
    ::android::hardware::Return<int32_t> setSidebandStream(const ::android::hardware::hidl_handle& stream) override;
    ::android::hardware::Return<void> allocateBuffers(uint32_t width, uint32_t height, ::android::hardware::graphics::common::V1_0::PixelFormat format, uint32_t usage) override;
    ::android::hardware::Return<int32_t> allowAllocation(bool allow) override;
    ::android::hardware::Return<int32_t> setGenerationNumber(uint32_t generationNumber) override;
    ::android::hardware::Return<void> getConsumerName(getConsumerName_cb _hidl_cb) override;
    ::android::hardware::Return<int32_t> setSharedBufferMode(bool sharedBufferMode) override;
    ::android::hardware::Return<int32_t> setAutoRefresh(bool autoRefresh) override;
    ::android::hardware::Return<int32_t> setDequeueTimeout(int64_t timeoutNs) override;
    ::android::hardware::Return<void> getLastQueuedBuffer(getLastQueuedBuffer_cb _hidl_cb) override;
    ::android::hardware::Return<void> getFrameTimestamps(getFrameTimestamps_cb _hidl_cb) override;
    ::android::hardware::Return<void> getUniqueId(getUniqueId_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;
    ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;
    ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;
    ::android::hardware::Return<void> setHALInstrumentation() override;
    ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;
    ::android::hardware::Return<void> ping() override;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;
    ::android::hardware::Return<void> notifySyspropsChanged() override;
    ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;

private:
    std::mutex _hidl_mMutex;
    std::vector<::android::sp<::android::hardware::hidl_binder_death_recipient>> _hidl_mDeathRecipients;
};

}  // namespace V1_0
}  // namespace bufferqueue
}  // namespace graphics
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_GRAPHICS_BUFFERQUEUE_V1_0_BPHWGRAPHICBUFFERPRODUCER_H
