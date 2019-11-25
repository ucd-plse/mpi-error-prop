#ifndef MESSAGE_GEOFF_GUARD
#define MESSAGE_GEOFF_GUARD 1

class MessageGeoff : public Message {
public:
  virtual string format();
  virtual string getErrorCodeStr(bool starBefore=false);

  // Allows header of all error codes to be printed before traces
  static string getGeoffErrorHeader();

private:
  typedef std::vector<std::pair<std::string, std::string>> propV;

  struct Node {
    string name;
    string type;
    string unique;
    propV properties;

    string format() const;
  };

  struct Relationship {
    string name;
    string type;
    propV properties;
    string unique;

    Relationship(string type) {
      this->type = type;
    }

    string format() const;
  };

  string formatGeoffRelationship(MessageGeoff::Node, MessageGeoff::Node, MessageGeoff::Relationship);
  static Node getSourceNode(string file, int line);
  static Node getECNode(string error_code);

  // Geoff relationships need to be connected to previous node
  static Node prev;
};

#endif

