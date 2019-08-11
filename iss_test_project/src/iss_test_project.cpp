#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <cassert>

#include <transmitter.h>
#include <consumer.h>

using iss_test::Transmitter;
using iss_test::Consumer;
using iss_test::Storage;




void check(std::shared_ptr<Storage> storage, Storage storage_expected)
{
  assert(storage->binary_packets_received_count_ == storage_expected.binary_packets_received_count_);
  assert(storage->text_packets_received_count_ == storage_expected.text_packets_received_count_);
  assert(storage->bytes_received_count_ == storage_expected.bytes_received_count_);
  assert(storage->buffer_ == storage_expected.buffer_);
}

int main()
{
  std::ios_base::sync_with_stdio(false);

  auto storage = std::make_shared<Storage>();
  storage->reset();
  auto transmitter = std::make_unique<Transmitter>(std::make_unique<Consumer>(storage));
  std::vector<char> buffer;
  std::vector<char> binary_packet = { 0x24, 0x05, 0x00, 0x00, 0x00, 0x24, '\r', '\n', '\r', '\n' };
  std::string text_packet = "12345\r\n89\r\n\r\n";

  // empty block
  transmitter->Receive(nullptr, 0);
  check(storage, { 0, 0, 0 });
  storage->reset();

  // empty text packet
  transmitter->Receive("\r\n\r\n", 4);
  check(storage, { 0, 1, 0 });
  storage->reset();

  // empty binary packet
  transmitter->Receive("$\x00\x00\x00\x00", 5);
  check(storage, { 1, 0, 0 });
  storage->reset();

  // split binary packet
  buffer = binary_packet;
  transmitter->Receive(&buffer[0], 3);
  check(storage, { 0, 0, 0 });
  transmitter->Receive(&buffer[0 + 3], 7);
  check(storage, { 1, 0, binary_packet.size() - 5, { binary_packet.begin() + 5, binary_packet.end() } });
  storage->reset();

  // split text packet
  buffer.assign(text_packet.begin(), text_packet.end());
  transmitter->Receive(&buffer[0], 8);
  check(storage, { 0, 0, 0 });
  transmitter->Receive(&buffer[0 + 8], 4);
  check(storage, { 0, 0, 0 });
  transmitter->Receive(&buffer[0 + 8 + 4], 1);
  check(storage, { 0, 1, text_packet.size() - 4, { text_packet.begin(), text_packet.end() - 4 } });
  storage->reset();

  // multi packet
  buffer.assign(text_packet.begin(), text_packet.end());
  buffer.insert(buffer.end(), text_packet.begin(), text_packet.end());
  buffer.insert(buffer.end(), binary_packet.begin(), binary_packet.end());
  buffer.insert(buffer.end(), text_packet.begin(), text_packet.end());
  buffer.insert(buffer.end(), binary_packet.begin(), binary_packet.end());
  buffer.insert(buffer.end(), binary_packet.begin(), binary_packet.end());

  std::vector<char> expected_multi_data(text_packet.begin(), text_packet.end() - 4);
  expected_multi_data.insert(expected_multi_data.end(), text_packet.begin(), text_packet.end() - 4);
  expected_multi_data.insert(expected_multi_data.end(), binary_packet.begin() + 5, binary_packet.end());
  expected_multi_data.insert(expected_multi_data.end(), text_packet.begin(), text_packet.end() - 4);
  expected_multi_data.insert(expected_multi_data.end(), binary_packet.begin() + 5, binary_packet.end());
  expected_multi_data.insert(expected_multi_data.end(), binary_packet.begin() + 5, binary_packet.end());

  transmitter->Receive(&buffer[0], static_cast<unsigned int>(buffer.size()));
  check(storage, { 3, 3, expected_multi_data.size(), expected_multi_data });
  storage->reset();

  // 100000 binary + text packets loop
  buffer = std::vector<char>(1024 + 5, 100);
  buffer[0] = 0x24;
  buffer[1] = 0x00;
  buffer[2] = 0x02;
  buffer[3] = 0x00;
  buffer[4] = 0x00;
  buffer.insert(buffer.end(), { '\r', '\n', '\r', '\n' });

  size_t loop_num = 100000;
  storage->buffer_.reserve(1024 * loop_num);

  std::chrono::steady_clock steady_clock;
  auto start = steady_clock.now();
  for (size_t i = 0; i < loop_num; i++)
  {
    transmitter->Receive(&buffer[0], static_cast<unsigned int>(buffer.size()));
  }
  auto end = steady_clock.now();

  expected_multi_data = std::vector<char>(1024 * loop_num, 100);
  check(storage, { loop_num, loop_num, expected_multi_data.size(), expected_multi_data });

  auto time_span = static_cast<std::chrono::duration<double>>(end - start);
  std::cout << "Operation took: " << time_span.count() << " seconds\n";
  std::cout << "Speed: " << storage->bytes_received_count_ / time_span.count() / 1024 / 1024 << " Mbit/s";

  return 0;
}
