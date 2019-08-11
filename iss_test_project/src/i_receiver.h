#ifndef ISS_TEST_PROJECT_IRECEIVER_H_
#define ISS_TEST_PROJECT_IRECEIVER_H_




namespace iss_test {

  struct IReceiver {
    virtual void Receive(const char* data, unsigned int size) = 0;
  };

}  // namespace iss_test

#endif  // ISS_TEST_PROJECT_IRECEIVER_H_