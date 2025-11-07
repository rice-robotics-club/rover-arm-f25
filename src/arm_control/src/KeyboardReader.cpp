// #include <X11/Xlib.h>
// #include <iostream>
// #include "X11/keysym.h"
// #include <cstdint>

// #include "rclcpp/rclcpp.hpp"
// #include "std_msgs/msg/int32.hpp"


// bool key_is_pressed(KeySym ks) {
//     Display *dpy = XOpenDisplay(":0");
//     char keys_return[32];
//     XQueryKeymap(dpy, keys_return);
//     KeyCode kc2 = XKeysymToKeycode(dpy, ks);
//     bool isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
//     XCloseDisplay(dpy);
//     return isPressed;
// }


// void construct_msg(*uint32_t msg) {
//     // we're gonna do something stupid
//     // setting up dumbass key map:
//     KeySym keysymlist[25] = {
//         XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7,
//         XK_8, XK_9, XK_w, XK_a, XK_s, XK_d, XK_t,
//         XK_f, XK_g, XK_h, XK_i, XK_j, XK_k, XK_l,
//         XK_z, XK_x, XK_n, XK_m
//     }
//     // now construct msg from keymap!
//     for (int i = 0; i < 25; i++) {
//         if key_is_pressed(keysymlist[i]) {
//             msg |= ((uint32_t)1 << i);
//         } else {
//             msg &= ~((uint32_t)1 << i);
//         }
//     }
//     for (int i = 25; i < 32; i++) {
//         msg &= ~((uint32_t)1 << i);
//     }
// }

// int main(int argc, char** argv){

//   // node init
//   rclcpp::init(argc,argv);
//   auto node = rclcpp::Node::make_shared("teleop");
//   // define publisher
//   auto _pub = node->create_publisher<std_msgs::msg::Int32>("/keyboard_data", 10);

//   std_msgs::msg::Int32 intdata;
  
//   while(rclcpp::ok()){
//     construct_msg(&intdata.data)
//     RCLCPP_INFO(node->get_logger(), "I ran!")
//     _pub->publish(intdata);
//     rclcpp::spin_some(node);  
//   }
//   return 0;

// }

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#define KEYCODE_R 0x43 
#define KEYCODE_L 0x44
#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_Q 0x71
#define KEYCODE_0 0x30
#define KEYCODE_1 0x31
#define KEYCODE_2 0x32
#define KEYCODE_3 0x33
#define KEYCODE_4 0x34
#define KEYCODE_5 0x35
#define KEYCODE_6 0x36
#define KEYCODE_7 0x37
#define KEYCODE_8 0x38
#define KEYCODE_9 0x39

class KeyboardReader
{
public:
  KeyboardReader();
  void keyLoop();

private:


  rclcpp::Node::SharedPtr nh_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;

};

KeyboardReader::KeyboardReader()
{
  nh_ = rclcpp::Node::make_shared("keyboard_reader");

  publisher_ = nh_->create_publisher<std_msgs::msg::String>("keyboard", 10);
}

int kfd = 0;
struct termios cooked, raw;

void quit(int sig)
{
  (void)sig;
  tcsetattr(kfd, TCSANOW, &cooked);
  rclcpp::shutdown();
  exit(0);
}


int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  KeyboardReader keyboard_reader;

  signal(SIGINT, quit);

  keyboard_reader.keyLoop();
  
  return(0);
}


void KeyboardReader::keyLoop()
{
  char c;
  bool dirty=false;


  // get the console in raw mode                                                              
  tcgetattr(kfd, &cooked);
  memcpy(&raw, &cooked, sizeof(struct termios));
  raw.c_lflag &=~ (ICANON | ECHO);
  // Setting a new line, then end of file                         
  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  tcsetattr(kfd, TCSANOW, &raw);

  puts("Reading from keyboard");
  puts("---------------------------");
  puts("Use arrow keys to move the turtle.");


  for(;;)
  {
    // get the next event from the keyboard  
    if(read(kfd, &c, 1) < 0)
    {
      perror("read():");
      exit(-1);
    }

    RCLCPP_DEBUG(nh_->get_logger(), "value: 0x%02X\n", c);

    std_msgs::msg::String str;
    str.data = "";
  
    switch(c)
    {
      case KEYCODE_L:
        RCLCPP_DEBUG(nh_->get_logger(), "LEFT");
        str.data += "L";
        dirty = true;
        break;
      case KEYCODE_R:
        RCLCPP_DEBUG(nh_->get_logger(), "RIGHT");
        str.data += "R";
        dirty = true;
        break;
      case KEYCODE_U:
        RCLCPP_DEBUG(nh_->get_logger(), "UP");
        str.data += "U";
        dirty = true;
        break;
      case KEYCODE_D:
        RCLCPP_DEBUG(nh_->get_logger(), "DOWN");
        str.data += "D";
        dirty = true;
        break;
      case KEYCODE_0:
        RCLCPP_DEBUG(nh_->get_logger(), "ZERO");
        str.data += "0";
        dirty = true;
        break;
      case KEYCODE_1:
        RCLCPP_DEBUG(nh_->get_logger(), "ONE");
        str.data += "1";
        dirty = true;
        break;
      case KEYCODE_2:
        RCLCPP_DEBUG(nh_->get_logger(), "TWO");
        str.data += "2";
        dirty = true;
        break;
      case KEYCODE_3:
        RCLCPP_DEBUG(nh_->get_logger(), "THREE");
        str.data += "3";
        dirty = true;
        break;
      case KEYCODE_4:
        RCLCPP_DEBUG(nh_->get_logger(), "FOUR");
        str.data += "4";
        dirty = true;
        break;
      case KEYCODE_5:
        RCLCPP_DEBUG(nh_->get_logger(), "FIVE");
        str.data += "5";
        dirty = true;
        break;
      case KEYCODE_6:
        RCLCPP_DEBUG(nh_->get_logger(), "SIX");
        str.data += "6";
        dirty = true;
        break;
      case KEYCODE_7:
        RCLCPP_DEBUG(nh_->get_logger(), "SEVEN");
        str.data += "7";
        dirty = true;
        break;
      case KEYCODE_8:
        RCLCPP_DEBUG(nh_->get_logger(), "EIGHT");
        str.data += "8";
        dirty = true;
        break;
      case KEYCODE_9:
        RCLCPP_DEBUG(nh_->get_logger(), "NINE");
        str.data += "9";
        dirty = true;
        break;
    }
   

    
    if(dirty ==true)
    {
      publisher_->publish(str);    
      dirty=false;
    }
  }


  return;
}

