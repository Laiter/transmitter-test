#include "transmitter.h"




namespace iss_test {

  void Transmitter::Receive(const char* data, unsigned int size) {
    if (!data || size == 0) return;
    input_data_.insert(input_data_.end(), data, data + size);
    while (processData());
  }

  PacketType Transmitter::evaluatePacketType() {
    if (current_packet_type_ == PacketType::kUnknown && input_data_.size() != 0)
    {
      if (input_data_[0] == 0x24) {
        current_packet_type_ = PacketType::kBinary;
        current_data_size_ = -1;
      }
      else {
        current_packet_type_ = PacketType::kText;
        current_data_size_ = 0;
      }
    }
    return current_packet_type_;
  }

  bool Transmitter::processData() {
    const char* data_ptr = nullptr;
    switch (evaluatePacketType()) {
    case PacketType::kBinary:
      if (!findBinaryPacketEnd()) return false;
      if (current_data_size_ != 0) data_ptr = &input_data_[kHeaderSize];
      callback_->BinaryPacket(data_ptr, static_cast<unsigned int>(current_data_size_));
      clearCurrentPacket(static_cast<unsigned int>(current_data_size_ + kHeaderSize));
      return true;
    case PacketType::kText:
      if (!findTextPacketEnd()) return false;
      if (current_data_size_ != 0) data_ptr = &input_data_[0];
      callback_->TextPacket(data_ptr, static_cast<unsigned int>(current_data_size_));
      clearCurrentPacket(static_cast<unsigned int>(current_data_size_ + sizeof(kTextPacketEndMask)));
      return true;
    default:
      return false;
    }
  }

  void Transmitter::clearCurrentPacket(unsigned int packet_size) {
    current_packet_type_ = PacketType::kUnknown;
    input_data_.assign(input_data_.begin() + packet_size, input_data_.end());
  }

  bool Transmitter::findTextPacketEnd() {
    if (input_data_.size() < 4) return false;
    auto found = std::search(input_data_.begin() + current_data_size_, input_data_.end(),
      std::begin(kTextPacketEndMask), std::end(kTextPacketEndMask));
    if (found != input_data_.end())
    {
      current_data_size_ = std::distance(input_data_.begin(), found);
      return true;
    }
    current_data_size_ = input_data_.size() - 3;
    return false;
  }

  bool Transmitter::findBinaryPacketEnd() {
    if (current_data_size_ == -1) {
      if (input_data_.size() < kHeaderSize) return false;
      current_data_size_ = static_cast<unsigned char>(input_data_[1])
        | (input_data_[2] << 8)
        | (input_data_[3] << 16)
        | (input_data_[4] << 24);
    }
    if (input_data_.size() >= static_cast<size_t>(current_data_size_) + kHeaderSize) {
      return true;
    }
    return false;
  }

}  // namespace iss_test