#ifndef ISS_TEST_PROJECT_CONSUMER_H_
#define ISS_TEST_PROJECT_CONSUMER_H_
#include <iostream>

#include "i_callback.h"
#include "packet_type.h"




namespace iss_test {

  struct Storage
  {
    void reset()
    {
      binary_packets_received_count_ = 0;
      text_packets_received_count_ = 0;
      bytes_received_count_ = 0;
      buffer_.clear();
    }

    size_t binary_packets_received_count_ = 0;
    size_t text_packets_received_count_ = 0;
    size_t bytes_received_count_ = 0;
    std::vector<char> buffer_;
  };

  class Consumer : public ICallback
  {
  public:
    Consumer(std::shared_ptr<Storage> storage) : storage_(storage) {}

    void BinaryPacket(const char* data, unsigned int size) override {
      processData(PacketType::kBinary, data, size);
    }

    void TextPacket(const char* data, unsigned int size) override {
      processData(PacketType::kText, data, size);
    }

  private:
    inline void processData(PacketType packet_type, const char* data, unsigned int size) {
      (packet_type == PacketType::kBinary
        ? storage_->binary_packets_received_count_ : storage_->text_packets_received_count_) += 1;
      storage_->bytes_received_count_ += size;
      storage_->buffer_.insert(storage_->buffer_.end(), data, data + size);
    }

    std::shared_ptr<Storage> storage_;
  };

}  // namespace iss_test

#endif  // ISS_TEST_PROJECT_CONSUMER_H_