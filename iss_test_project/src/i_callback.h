#ifndef ISS_TEST_PROJECT_ICALLBACK_H_
#define ISS_TEST_PROJECT_ICALLBACK_H_




namespace iss_test {

  struct ICallback {
    virtual void BinaryPacket(const char* data, unsigned int size) = 0;
    virtual void TextPacket(const char* data, unsigned int size) = 0;
  };

}  // namespace iss_test

#endif  // ISS_TEST_PROJECT_IRECEIVER_H_