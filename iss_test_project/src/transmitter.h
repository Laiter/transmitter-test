#ifndef ISS_TEST_PROJECT_TRANSMITTER_H_
#define ISS_TEST_PROJECT_TRANSMITTER_H_
#include <vector>
#include <memory>
#include <algorithm>

#include "i_receiver.h"
#include "i_callback.h"
#include "packet_type.h"




namespace iss_test {

  class Transmitter : public IReceiver
  {
  public:
    Transmitter(std::unique_ptr<ICallback> callback) :
      callback_(std::move(callback)),
      current_packet_type_(PacketType::kUnknown),
      current_data_size_(-1)
    {
      input_data_.reserve(4 * 1024);
    }

    void Receive(const char* data, unsigned int size) override;

  private:
    static constexpr uint8_t kHeaderSize = 5;
    static constexpr const char kTextPacketEndMask[]{ '\r','\n','\r','\n' };

    PacketType evaluatePacketType();
    bool processData();
    void clearCurrentPacket(unsigned int packet_size);
    bool findTextPacketEnd();
    bool findBinaryPacketEnd();

    std::unique_ptr<ICallback> callback_;
    std::vector<char> input_data_;
    PacketType current_packet_type_;
    long long current_data_size_;
  };

}  // namespace iss_test

#endif  // ISS_TEST_PROJECT_TRANSMITTER_H_