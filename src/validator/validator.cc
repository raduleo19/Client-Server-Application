#include "validator.hh"

// Return first error message
string Validate(std::string command) {
  istringstream command_stream(command);
  command_stream >> command;
  if (command == "subscribe") {
    string topic;
    string sf_flag;
    command_stream >> topic >> sf_flag;
    if (topic.size() > 50) {
      return "Topic length exceded. Max 50 characters.";
    } else if (topic.empty()) {
      return "Missing topic.";
    } else if (sf_flag.empty()) {
      return "Missing SF flag.";
    } else if (sf_flag != "0" && sf_flag != "1") {
      return "Invalid SF. Must be 0 or 1.";
    }
    string garbage;
    if (command_stream >> garbage) {
      return "Invalid command. Commands: "
             "subscribe <topic> <sf>, "
             "unsubscribe <topic>, "
             "exit.";
    }
  } else if (command == "unsubscribe") {
    string topic;
    command_stream >> topic;
    if (topic.size() > 50) {
      return "Topic length exceded. Max 50 characters.";
    } else if (topic.empty()) {
      return "Missing topic.";
    }
    string garbage;
    if (command_stream >> garbage) {
      return "Invalid command. Commands: subscribe <topic> <sf>, "
             "unsubscribe <topic> and exit.";
    }
  } else if (command == "exit") {
    string garbage;
    if (command_stream >> garbage) {
      return "Invalid command. Commands: subscribe <topic> <sf>, "
             "unsubscribe <topic> and exit.";
    }
    return "";
  } else {
    return "Invalid command. Commands: subscribe <topic> <sf>, "
           "unsubscribe <topic> and exit.";
  }
  return "";
}