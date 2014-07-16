
#ifndef PROMPT_PLAYER_
#define PROMPT_PLAYER_


#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vlc/vlc.h>

using std::mutex;
using std::string;

namespace EHwA {

class PromptPlayer {
public:
  PromptPlayer();
  ~PromptPlayer();
  void Play(string media_filename);
  bool TryLock();
  
protected:
  libvlc_instance_t * inst_;
  
  // multi-threading
  mutex mtx_playing_;
  
};

}  // namespace EHwA

#endif  // PROMPT_PLAYER_
