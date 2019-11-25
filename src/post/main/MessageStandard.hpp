#ifndef MESSAGE_STANDARD_GUARD
#define MESSAGE_STANDARD_GUARD 1

class MessageStandard : public Message {
public:
  virtual string format();
  virtual string getErrorCodeStr(bool starBefore=false);
private:
  string getLocationStr();
};

#endif

