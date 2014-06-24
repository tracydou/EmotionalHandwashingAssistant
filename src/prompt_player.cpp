
#include "prompt_player.hpp"
namespace EHwA {
	
PromptPlayer::PromptPlayer() {
  /* Load the VLC engine */
  inst_ = libvlc_new (0, NULL);
}

PromptPlayer::~PromptPlayer() {
  libvlc_release(inst_);
}

void PromptPlayer::Play(string media_filename) {
  // Set up media & media-player
  libvlc_media_t *m = libvlc_media_new_path (inst_, media_filename.c_str());
  libvlc_media_player_t *mp = libvlc_media_player_new_from_media (m);
  // Play the media player; let is play for a while and then stop playing
  // TODO: how to decide how long to "sleep()"
  libvlc_media_player_play (mp);
  sleep(10);
  libvlc_media_player_stop (mp);
  // Release media & media-player  
  libvlc_media_release (m);
  libvlc_media_player_release (mp);
}
 
// This is a non working code that show how to hooks into a window,
// if we have a window around
// libvlc_media_player_set_xwindow (mp, xid);
 
}  // namespace EHwA
