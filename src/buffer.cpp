/*
 *  File:				buffer.cpp
 *  Created by:			Luyuan Lin
 * 
 */
 
 #include "buffer.hpp"
 
namespace EHwA {
  
  const int Buffer::STATE_A = 0;
  const int Buffer::STATE_B = 1;
  const int Buffer::STATE_C = 2;
  const double Buffer::ALPHA = 1;
  
  Buffer::Buffer(double threshold_timeout, double threshold_timeup) :
    threshold_timeout_(threshold_timeout), threshold_timeup_(threshold_timeup) {
    current_state_ = STATE_A;
    time_count_A_ = 0;
    time_count_B_ = 0;
    behaviour_state_A_ = BAYESACT_NOTHING; // default value
    behaviour_state_B_ = BAYESACT_NOTHING; // default value
    current_epa_.resize(3);
  }
  
  Buffer::~Buffer() {
    // do nothing
  }
  
  int Buffer::get_current_state() {
    return current_state_;
  }
  
  int Buffer::get_current_user_behaviour() {
    if (current_state_ == STATE_B) {
      return behaviour_state_B_;
    } else {
      return behaviour_state_A_;
    }
  }

  vector<double> Buffer::get_current_epa() {
    return current_epa_;
  }

  // called at analyzer for each frame
  void Buffer::Update(int hand_action, const Position& left_hand_pos,
               const Position& right_hand_pos) {
    // State changes according to hand_action recognized
    if (current_state_ == STATE_A) {
        if (hand_action == BAYESACT_NOTHING || hand_action == behaviour_state_A_) {
            time_count_A_ ++;
            if (time_count_A_ == threshold_timeout_) {
                cout << "[loginfo:] ---------- timeout!" << endl;
                ChangeToState(STATE_C, behaviour_state_A_);
            }
        } else {
            ChangeToState(STATE_B, hand_action);
        }
    } else if (current_state_ == STATE_B) {
        if (hand_action == behaviour_state_B_) {
            time_count_B_ ++;
            if (time_count_B_ == threshold_timeup_) {
                behaviour_state_A_ = behaviour_state_B_;
                ChangeToState(STATE_C, behaviour_state_B_);
            }
        } else {
            ChangeToState(STATE_A, hand_action);
        }
    }
    
    // update current_epa_ 
    hand_positions_.push_front(
            pair<Position, Position> (left_hand_pos, right_hand_pos));
    // NUM_POSITIONS_NEEDED, defined in defines.hpp, is the number of 
    // handpositions needed by epa-calc to compute epa values
    if (hand_positions_.size() > NUM_POSITIONS_NEEDED) {
        hand_positions_.pop_back();
    }
    vector<double> previous_epa(current_epa_);
    vector<double> tmp_epa = EPACalculator::Calculate(hand_positions_);
    for (int i = 0; i < 3; ++i) {
        current_epa_[i] = (ALPHA * previous_epa[i] + tmp_epa[i]) / (ALPHA + 1);
    }
  }
               
  void Buffer::ChangeToState(int new_state, int new_behaviour) {
    if (new_state == STATE_C) { // current behaviour already been assigned to behaviour_state_A_
        time_count_A_ = 0;
        time_count_B_ = 0;
        current_state_ = STATE_C;
    } else if (new_state == STATE_A) {
        if (current_state_ == STATE_C) {
            time_count_A_ = 0;
            behaviour_state_A_ = new_behaviour;
            current_state_ = STATE_A;
        } else { // current_state_ == STATE_B
            if (time_count_A_ + time_count_B_ >= threshold_timeout_) {
                if (time_count_A_ > time_count_B_) {
                    ChangeToState(STATE_C, behaviour_state_A_);
                } else {
                    ChangeToState(STATE_C, behaviour_state_B_);
                }
            } else {
                time_count_A_ += time_count_B_;
                current_state_ = STATE_A;
            }
        }
    } else { // new_state == STATE_B, current_state_ == STATE_A
        time_count_B_ = 0;
        behaviour_state_B_ = new_behaviour;
        current_state_ = STATE_B;
    }
  }
  
}  // namespace EHwA
 
 
