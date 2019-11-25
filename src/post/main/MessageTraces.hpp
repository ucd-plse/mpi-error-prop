#ifndef MESSAGE_TRACES
#define MESSAGE_TRACES

class MessageTraces : public Message {
public:
  virtual string format();
  virtual string getErrorCodeStr(bool starBefore=false);
private:
  string getLocationStr();
};

#endif

