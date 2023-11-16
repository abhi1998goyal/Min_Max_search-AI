#include <stack>
#pragma once
#include "engine_base.hpp"
#include <atomic>

class Engine : public AbstractEngine {

    // add extra items here. 
    // Note that your engine will always be instantiated with the default 
    // constructor.
    
    public:
    void find_best_move(const Board& b) override;
    U16 find_best_move_7_3(const Board& b);
    U16 find_best_move_8_4(const Board &b);
    U16 find_best_move_8_2(const Board &b);
    std::unordered_set<U16> get_legal_moves_piece(Board c , U8 piece_pos);
    int no_start_moves_7_3=7;
    U16 start_moves_7_3[7]={0x0A11,0x1119,0x0209,0x030A,0x0401,0x0C0D,0x0902};
    int no_start_moves_8_4=9;
    U16 start_moves_8_4[10]={0x0A11,0x1119,0x0209,0x030A,0x0400,0xC04,0x0401,0x0515,0x902};
    int no_start_moves_8_2=8;
    U16 start_moves_8_2[8]={0x1319,0x0C13,0x150C,0x0B15,0x0209,0x0500,0x0D05,0x0501};
    std::stack<U16> check_mate_moves;
    //std::unordered_map<std::string,std::pair<int,int>> state_win_prob_map;
   // std::unordered_map<std::string,std::vector<string>> state_child_visit_map;
};
