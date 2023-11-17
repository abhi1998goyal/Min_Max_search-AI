#include <algorithm>
#include <random>
#include <iostream>
#include <thread>
#include <fstream>
#include <chrono>
//#include "CoutRedirector.cpp"
#include "engine.hpp"
#include "butils.hpp"

static std::unordered_map<uint32_t, int> boardCounts;
std::unordered_map<uint32_t, std::string> hashedStrings;

enum PieceValue {
    PAWN_VALUE   = 3,
    ROOK_VALUE   = 7,
    BISHOP_VALUE = 11,
    KNIGHT_VALUE = 5
};

uint32_t FNV_PRIME = 16777619;
uint32_t FNV_OFFSET_BASIS = 2166136261;

uint32_t uniq_hash(const std::string& str) {
    uint32_t hash = 2166136261;
    for (char c : str) {
        hash ^= uint32_t(c);
        hash *= 16777619;
    }
    hashedStrings[hash] = str;
    return hash;
}

std::vector<U16> random_sampling(auto moveset){
    std::vector<U16> moves;
    std::sample(
            moveset.begin(),
            moveset.end(),
            std::back_inserter(moves),
            1,
            std::mt19937{std::random_device{}()}
        );
    return moves;
}



int count_pieces(const Board&b ,U8 piece){
    int count =0;
    for(int i=0;i<63;i++){
           if(b.data.board_0[i]==piece){
              count++;
        }
    }
   return count;
}

double center_eval(const Board&b,PlayerColor player){
    std::vector<std::pair<int,int>> center_squares = { 
                                                       {1, 2}, {1,3}, {1,4},
                                                       {5, 2}, {5,3}, {5,4},
                                                       }; 
                                                      // {6, 2}, {6,3}, {6,4} //{0, 2}, {0,3}, {0,4},
    double count=0;
     for(std::pair<int,int> sq:center_squares){
        if((b.data.board_0[pos(sq.first,sq.second)]&BLACK)==player){
              count++;
        }
        else if((b.data.board_0[pos(sq.first,sq.second)]&WHITE)==player){
              count++;
        }
     }
     return count;
}

double center_eval_8_4(const Board&b,PlayerColor player){
    std::vector<std::pair<int,int>> center_squares = { 
                                                       {1, 2}, {1,3}, {1,4},{1,5},
                                                       {5, 2}, {5,3}, {5,4},{5,5}
                                                       }; 
                                                      // {6, 2}, {6,3}, {6,4} //{0, 2}, {0,3}, {0,4},
    double count=0;
     for(std::pair<int,int> sq:center_squares){
        if((b.data.board_0[pos(sq.first,sq.second)]&BLACK)==player){
              count++;
        }
        else if((b.data.board_0[pos(sq.first,sq.second)]&WHITE)==player){
              count++;
        }
     }
     return count;
}

double center_eval_8_2(const Board&b,PlayerColor player){
    std::vector<std::pair<int,int>> center_squares = { 
                                                       {1, 2}, {1,3}, {1,4},{1,5},
                                                       {2, 2}, {2,3}, {2,4},{2,5},
                                                       {5, 2}, {5,3}, {5,4},{5,5},
                                                       {6, 2}, {6,3}, {6,4},{6,5}
                                                       }; 
                                                      // {6, 2}, {6,3}, {6,4} //{0, 2}, {0,3}, {0,4},
    double count=0;
     for(std::pair<int,int> sq:center_squares){
        if((b.data.board_0[pos(sq.first,sq.second)]&BLACK)==player){
              count++;
        }
        else if((b.data.board_0[pos(sq.first,sq.second)]&WHITE)==player){
              count++;
        }
     }
     return count;
}

double pawn_eval(const Board&b,PlayerColor player){
     std::vector<std::pair<int,int>> end_square_b = {{4,5},{4,6}};
     std::vector<std::pair<int,int>> end_square_w = {{2,0},{2,1}};
     std::vector<std::pair<int,int>> end_square=end_square_b;
     if(player==WHITE){
        end_square=end_square_w;
     }
     double count=0;
     for(std::pair<int,int> sq:end_square){
        if(b.data.board_0[pos(sq.first,sq.second)]==(b.data.player_to_play|PAWN)){
              count++;
        }
     }
     return count;
}

double pawn_eval_8_4(const Board&b,PlayerColor player){
     std::vector<std::pair<int,int>> end_square_w = {{5,6},{5,7}};
     std::vector<std::pair<int,int>> end_square_b = {{2,0},{2,1}};
     std::vector<std::pair<int,int>> end_square=end_square_b;
     if(player==WHITE){
        end_square=end_square_w;
     }
     double count=0;
     for(std::pair<int,int> sq:end_square){
        if(b.data.board_0[pos(sq.first,sq.second)]==(b.data.player_to_play|PAWN)){
              count++;
        }
     }
     return count;
}

double pawn_eval_8_2(const Board&b,PlayerColor player){
     std::vector<std::pair<int,int>> end_square_w = {{4,5},{4,6},{4,7}};
     std::vector<std::pair<int,int>> end_square_b = {{3,0},{3,1},{3,2}};
     std::vector<std::pair<int,int>> end_square=end_square_b;
     if(player==WHITE){
        end_square=end_square_w;
     }
     double count=0;
     for(std::pair<int,int> sq:end_square){
        if(b.data.board_0[pos(sq.first,sq.second)]==(b.data.player_to_play|PAWN)){
              count++;
        }
     }
     return count;
}

int count_pieces(const Board&b){
     int count =0;
    for(int i=0;i<63;i++){
           if((b.data.board_0[i] & b.data.player_to_play)==b.data.player_to_play){
              count++;
        }
    }
   return count;
}

bool is_stalemate(const Board&b){
     return b.get_legal_moves().size()<=0?true:false;
}

std::unordered_set<U16> Engine::get_legal_moves_piece(Board c , U8 piece_pos){
    auto pseudolegal_moves = c.get_pseudolegal_moves_for_piece(piece_pos);
    std::unordered_set<U16> legal_moves;
    for (auto move : pseudolegal_moves) {
        c.do_move_without_flip_(move); 
        if (!c.in_check()) {
            legal_moves.insert(move);
        }
        else {
            std::cout << "Move " << move_to_str(move) << " is illegal" << std::endl;
        }
        c.undo_last_move_without_flip_(move);
    }
    return legal_moves;
}

double dist_from_king(Board b,PlayerColor opponent){
     int kingx, kingy;
     if (opponent == BLACK){
        kingx = getx(b.data.b_king);
        kingy = gety(b.data.b_king);
    }
    else{
        kingx = getx(b.data.w_king);
        kingy = gety(b.data.w_king);
    }
     int si = (opponent ^ (WHITE | BLACK)>>7) * 10;
    double dist=0;
    U8 *pieces = (U8*)(&b.data);
    int non_dead=0;
    for (int i=0; i<b.data.n_pieces; i++) {
        if (pieces[si+i] == DEAD) {
            continue;
        }
        non_dead++;
        dist+=(abs(kingx-getx(pieces[si+i])) + abs(kingx-getx(pieces[si+i])));

    }
   if (non_dead == 0) {
        return 0;  // Avoid division by zero
    }

    double average_distance = dist / non_dead;
    double mean = 3.0;  // Desired mean distance

    double sigma = 4.0;
    double exponent = -0.5 * ((average_distance - mean) / sigma) * ((average_distance - mean) / sigma);
    double reward = exp(exponent);

    return reward;
}

bool all_except_king_dead(const Board &b,PlayerColor player){
    if(player==BLACK){
         if(b.data.b_bishop==DEAD && b.data.b_knight_1==DEAD && b.data.b_knight_2==DEAD && 
            b.data.b_rook_1==DEAD && b.data.b_rook_2==DEAD){
                return true;
            }
    }
    else{
        if(b.data.w_bishop==DEAD && b.data.w_knight_1==DEAD && b.data.w_knight_2==DEAD && 
            b.data.w_rook_1==DEAD && b.data.w_rook_2==DEAD){
                return true;
            }
    }
   return false;
}

double eval_7_4(const Board& b,PlayerColor player){
      double score = 0;
    U8 me,opponent ;
       me=player;
       opponent=player^(BLACK|WHITE);
    if(boardCounts[uniq_hash(board_to_str(&b.data))]<2){
        Board* copy_board = new Board(b); //current player in check
        copy_board->data.player_to_play = (PlayerColor)opponent;
        score += PAWN_VALUE * (count_pieces(b, me|PAWN) - count_pieces(b, opponent|PAWN));
        score += ROOK_VALUE * (count_pieces(b, me|ROOK) - count_pieces(b, opponent|ROOK));
        score += BISHOP_VALUE * (count_pieces(b, me|BISHOP) - count_pieces(b, opponent|BISHOP));
       // score +=(center_eval(b,player)-center_eval(b,(PlayerColor)opponent))/6.0;
        score +=(((double)b.get_legal_moves().size()-(double)(*copy_board).get_legal_moves().size())/50.0);  //mobility evaluation
     //   score +=(pawn_eval(b,player)-pawn_eval(b,(PlayerColor)opponent))/2.0;
        score -= b.in_check()?3:0;
        score+=(*copy_board).in_check()?3:0;  //opponent in check
        score-=((((*copy_board).get_legal_moves().size()<=0) && !((*copy_board).in_check()))?50:0);//probably stalemate
        score+=(((*copy_board).in_check()&&(*copy_board).get_legal_moves().size()<=0)?300:0); //checkmate
        int mul = all_except_king_dead(b,(PlayerColor)opponent)?5:2;
        score+=mul*(dist_from_king(b,(PlayerColor)opponent));
    }
    else{
        score-=100;//3 fold draw
    }
    std::cout<<board_to_str(&b.data)<<" score "<<score<<"\n";
    return score;
}

double eval_8_4(const Board& b,PlayerColor player){
      double score = 0;
    U8 me,opponent ;
       me=player;
       opponent=player^(BLACK|WHITE);
    if(boardCounts[uniq_hash(board_to_str(&b.data))]<2){
        Board* copy_board = new Board(b); //current player in check
        copy_board->data.player_to_play = (PlayerColor)opponent;
        score += PAWN_VALUE * (count_pieces(b, me|PAWN) - count_pieces(b, opponent|PAWN));
        score += ROOK_VALUE * (count_pieces(b, me|ROOK) - count_pieces(b, opponent|ROOK));
        score += BISHOP_VALUE * (count_pieces(b, me|BISHOP) - count_pieces(b, opponent|BISHOP));
        score +=(center_eval_8_4(b,player)-center_eval_8_4(b,(PlayerColor)opponent))/8.0;
        score +=(((double)b.get_legal_moves().size()-(double)(*copy_board).get_legal_moves().size())/50.0);  //mobility evaluation
        score +=(pawn_eval_8_4(b,player)-pawn_eval_8_4(b,(PlayerColor)opponent))/2.0;
        score -= b.in_check()?20:0;
        score+=(*copy_board).in_check()?3:0;  //opponent in check
        score-=((((*copy_board).get_legal_moves().size()<=0) && !((*copy_board).in_check()))?50:0);//probably stalemate
        score+=(((*copy_board).in_check()&&(*copy_board).get_legal_moves().size()<=0)?300:0); //checkmate
        int mul = all_except_king_dead(b,(PlayerColor)opponent)?5:2;
        score+=mul*(dist_from_king(b,(PlayerColor)opponent));
    }
    else{
        score-=100;//3 fold draw
    }
    std::cout<<board_to_str(&b.data)<<" score "<<score<<"\n";
    return score;
}

double eval_8_2(const Board& b,PlayerColor player){
      double score = 0;
    U8 me,opponent ;
       me=player;
       opponent=player^(BLACK|WHITE);
    if(boardCounts[uniq_hash(board_to_str(&b.data))]<2){
        Board* copy_board = new Board(b); //current player in check
        copy_board->data.player_to_play = (PlayerColor)opponent;
        score += PAWN_VALUE * (count_pieces(b, me|PAWN) - count_pieces(b, opponent|PAWN));
        score += ROOK_VALUE * (count_pieces(b, me|ROOK) - count_pieces(b, opponent|ROOK));
        score += BISHOP_VALUE * (count_pieces(b, me|BISHOP) - count_pieces(b, opponent|BISHOP));
        score += KNIGHT_VALUE * (count_pieces(b, me|KNIGHT) - count_pieces(b, opponent|KNIGHT));
        score +=(center_eval_8_2(b,player)-center_eval_8_2(b,(PlayerColor)opponent))/8.0;
        score +=(((double)b.get_legal_moves().size()-(double)(*copy_board).get_legal_moves().size())/50.0);  //mobility evaluation
        score +=(pawn_eval_8_2(b,player)-pawn_eval_8_2(b,(PlayerColor)opponent))/4.0;
        score -= b.in_check()?20:0;
        score+=(*copy_board).in_check()?3:0;  //opponent in check
        score-=((((*copy_board).get_legal_moves().size()<=0) && !((*copy_board).in_check()))?50:0);//probably stalemate
        score+=(((*copy_board).in_check()&&(*copy_board).get_legal_moves().size()<=0)?300:0); //checkmate
        int mul = all_except_king_dead(b,(PlayerColor)opponent)?5:2;
        score+=mul*(dist_from_king(b,(PlayerColor)opponent));
    }
    else{
        score-=100;//3 fold draw
    }
    std::cout<<board_to_str(&b.data)<<" score "<<score<<"\n";
    return score;
}

double evaluate(const Board& b,PlayerColor player) {
   if(b.data.board_type==SEVEN_THREE){
      return eval_7_4(b,player);
   }
   else if(b.data.board_type==EIGHT_FOUR){
      return eval_8_4(b,player);
   }
   else 
    {
      return eval_8_2(b,player);
    }

}

double alpha_beta_minimax(Board b, int depth, double alpha, double beta, bool maximizing_player,Engine *e,PlayerColor player) {

    if (depth <= 0 || b.get_legal_moves().size()<=0) {
        return evaluate(b,player);
    }

    if (maximizing_player) {
        double max_eval = -99999;
        U16 best_move=0;
        auto moves = b.get_legal_moves();
        for (auto move : moves) {
            Board* copy_board = new Board(b);
            copy_board->do_move_(move);
          //  copy_board->data.player_to_play = (PlayerColor)(copy_board->data.player_to_play ^ (WHITE | BLACK));
            double eval = alpha_beta_minimax(*copy_board, depth - 1, alpha, beta, false,e,player);
            max_eval = std::max(max_eval, eval);
            if(eval>max_eval){
                best_move=move;
                max_eval=eval;
            }
            alpha = std::max(alpha, eval);

            if (beta <= alpha) {
                break;
            }
        }
        if(max_eval>300 && best_move!=0 && b.data.player_to_play==player){
            (*e).check_mate_moves.push(best_move);
        }
          
        return max_eval;
    } else {
        double min_eval = 99999;
        auto moves = b.get_legal_moves();
        U16 best_move=0;
        for (auto move : moves) {
            Board* copy_board = new Board(b);
            copy_board->do_move_(move);
           // copy_board->data.player_to_play = (PlayerColor)(copy_board->data.player_to_play ^ (WHITE | BLACK));
            double eval = alpha_beta_minimax(*copy_board, depth - 1, alpha, beta, true,e,player);
           // min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if(eval<min_eval){
                best_move=move;
                min_eval=eval;
            }

            if (beta <= alpha) {
                break;
            }
        }
        if(min_eval>300 && best_move!=0 && b.data.player_to_play==player){
            (*e).check_mate_moves.push(best_move);
        }
    
        return min_eval;
    }
}


bool no_immidiate_threat(const Board &b){
    std::ofstream outdata; 
    outdata.open("log.txt", std::ios_base::app);
    int si = (b.data. player_to_play>>7) * 10;
    U8 *pieces = (U8*)(&b.data);
     for (int i=0; i<b.data.n_pieces; i++) {
        U8 piece = pieces[si+i];
        if (piece == DEAD) continue;
        else {
            if(b.under_threat(piece)){
                outdata<<" Threat found"<<std::endl;
                return false;
            }
        }
    }
   return true;
}

U16  get_anti_move(Board b,U16 move){
    int max_x , max_y;
    if(b.data.board_type==SEVEN_THREE){
       max_x=6;
       max_y=6;
    }
    else {
       max_x=7;
       max_y=7;
    }

    U8 des = getp1(move);
    U8 src = getp0(move);
    U16 des_new= 0x0000;
    des_new+=(max_x-getx(des));
    des_new+=((max_y-gety(des))<<3);
    U16 src_new= 0x0000;
    src_new+=(max_x-getx(src));
    src_new+=((max_y-gety(src))<<3);
    U16 x = des_new + (src_new<<8);
    
 return x;
}

U16 Engine::find_best_move_7_3(const Board &b){
     std::ofstream outdata; 
    outdata.open("log.txt", std::ios_base::app);
     std::unordered_set<U16> moveset = b.get_legal_moves();
     Board *b1 = new Board(b);
     if (moveset.size() == 0) {
        std::cout << "Could not get any moves from board!\n";
        std::cout << board_to_str(&b.data);
        this->best_move = 0;
    }
    bool is_legal_move=false;

    std::cout<<"1"<<std::endl;
    //outdata<<"1";
     if(this->no_start_moves_7_3!=0){
        if(b1->data.player_to_play==BLACK){
            U16 anti_move= get_anti_move(*b1,this->start_moves_7_3[7-this->no_start_moves_7_3]);
            outdata<<"Anti move"<< move_to_str(anti_move)<<std::endl;
            is_legal_move=moveset.count(anti_move)>0?true:false;
            b1->do_move_without_flip_(get_anti_move(*b1,this->start_moves_7_3[7-this->no_start_moves_7_3])); //correct this
        }
        else{
            is_legal_move=moveset.count(this->start_moves_7_3[7-this->no_start_moves_7_3])>0?true:false;
            b1->do_move_without_flip_(this->start_moves_7_3[7-this->no_start_moves_7_3]); 
        }
     }
     if(this->no_start_moves_7_3!=0 && b.data.player_to_play==WHITE && no_immidiate_threat(b) && no_immidiate_threat(*b1) && is_legal_move){
            this->best_move=this->start_moves_7_3[7-this->no_start_moves_7_3];
            this->no_start_moves_7_3--;
            return this->best_move;
        }
     else if(this->no_start_moves_7_3!=0 && b.data.player_to_play==BLACK && no_immidiate_threat(b) && no_immidiate_threat(*b1) && is_legal_move){
            this->best_move= get_anti_move(b,this->start_moves_7_3[7-this->no_start_moves_7_3]);
            this->no_start_moves_7_3--;
            this->best_move;
     }
    else {
        std::cout<<"2"<<std::endl;
        int depth_level=3; //4
        //if(!no_immidiate_threat(b)){
          //  depth_level=2;
        //}
     boardCounts[uniq_hash(board_to_str(&b.data))]++;
     std::unordered_set<U16> special_moves; 
     auto moveset = b.get_legal_moves();
      if(b.data.player_to_play==BLACK){
         std::unordered_set<U16> special_moves_knight_b_1 = b.get_pseudolegal_moves_for_piece(b.data.b_knight_1);
         std::unordered_set<U16> special_moves_knight_b_2 = b.get_pseudolegal_moves_for_piece(b.data.b_knight_2);
         std::unordered_set<U16> special_moves_bishop_b = b.get_pseudolegal_moves_for_piece(b.data.b_bishop);
         special_moves.insert(special_moves_knight_b_1.begin(), special_moves_knight_b_1.end());
         special_moves.insert(special_moves_knight_b_2.begin(), special_moves_knight_b_2.end());
         special_moves.insert(special_moves_bishop_b.begin(),special_moves_bishop_b.end());
       }

     else if (b.data.player_to_play==WHITE){
         std::unordered_set<U16> special_moves_knight_w_1 = b.get_pseudolegal_moves_for_piece(b.data.w_knight_1);
         std::unordered_set<U16> special_moves_knight_w_2 = b.get_pseudolegal_moves_for_piece(b.data.w_knight_1);
         std::unordered_set<U16> special_moves_bishop_w = b.get_pseudolegal_moves_for_piece(b.data.w_bishop);
         special_moves.insert(special_moves_knight_w_1.begin(), special_moves_knight_w_1.end());
         special_moves.insert(special_moves_knight_w_2.begin(), special_moves_knight_w_2.end());
         special_moves.insert(special_moves_bishop_w.begin(),special_moves_bishop_w.end());
       }
        std::unordered_set<U16> last_moves=b.get_pseudolegal_moves_for_piece(this->last_moved);
        special_moves.insert( last_moves.begin(), last_moves.end());
    
     std::cout <<"Legal Moves :";
     for (auto m : moveset) {
            std::cout << move_to_str(m) << " ";
        }
      std::cout <<" Special Moves :";
     for (auto m : special_moves) {
           outdata << move_to_str(m) << " ";
        }
    std::cout <<"\n";
    double best_score = -99999;
    U16 best_move = random_sampling(moveset)[0];
    this->best_move=best_move;
    std::cout<<"random_move "<<move_to_str(this->best_move)<<"\n";
    std::chrono::milliseconds time_limit(20000);
    //Board* copy_board = b.copy();
    for(int depth=depth_level;depth<=depth_level;depth++){
        double best_max = -99999;
        U16 max_move=0;
        bool complete=true;
        std::cout<<"at depth "<<depth;
        for (auto move : moveset)
        {   if(special_moves.count(move)>0 && this->time_left>time_limit){
              depth = 4;
        }   if(this->time_left<time_limit){
              depth =2 ;
        }
            Board *copy_board = new Board(b);
            copy_board->do_move_(move);
          //  copy_board->data.player_to_play = (PlayerColor)(copy_board->data.player_to_play ^ (WHITE | BLACK));
            double score = alpha_beta_minimax(*copy_board, depth - 1, -99999, 99999, false, this,b.data.player_to_play);
          //  std::cout<<board_to_str(&b.data)<<" score "<<score<<"\n";
            outdata<<board_to_str(&b.data)<<" score "<<score<<" "<<depth<<" "<<move_to_str(move)<<"\n";

            if (score > best_max)
            {
                best_max = score;
                max_move = move;
            }

        }
           best_score=best_max;
           best_move=max_move;

        std::cout<<" best move "<<move_to_str(best_move)<<"\n";
      //  outdata<<" best move "<<move_to_str(best_move)<<"\n";
        this->best_move=best_move;
    }
   this->best_move = best_move;
        }
    return best_move;
}


U16 Engine::find_best_move_8_4(const Board &b){
     std::ofstream outdata; 
    outdata.open("log.txt", std::ios_base::app);
     std::unordered_set<U16> moveset = b.get_legal_moves();
     Board *b1 = new Board(b);
     if (moveset.size() == 0) {
        std::cout << "Could not get any moves from board!\n";
        std::cout << board_to_str(&b.data);
        this->best_move = 0;
    }
    bool is_legal_move=false;

    std::cout<<"1"<<std::endl;
    //outdata<<"1";
     if(this->no_start_moves_8_4!=0){
        if(b1->data.player_to_play==BLACK){
            U16 anti_move= get_anti_move(*b1,this->start_moves_8_4[9-this->no_start_moves_8_4]);
            outdata<<"Anti move"<< move_to_str(anti_move)<<std::endl;
            is_legal_move=moveset.count(anti_move)>0?true:false;
            b1->do_move_without_flip_(get_anti_move(*b1,this->start_moves_8_4[9-this->no_start_moves_8_4])); //correct this
        }
        else{
            is_legal_move=moveset.count(this->start_moves_8_4[9-this->no_start_moves_8_4])>0?true:false;
            b1->do_move_without_flip_(this->start_moves_8_4[9-this->no_start_moves_8_4]); 
        }
     }
     if(this->no_start_moves_8_4!=0 && b.data.player_to_play==WHITE && no_immidiate_threat(b) && no_immidiate_threat(*b1) && is_legal_move){
            this->best_move=this->start_moves_8_4[9-this->no_start_moves_8_4];
            this->no_start_moves_8_4--;
            return this->best_move;
        }
     else if(this->no_start_moves_8_4!=0 && b.data.player_to_play==BLACK && no_immidiate_threat(b) && no_immidiate_threat(*b1) && is_legal_move){
            this->best_move= get_anti_move(b,this->start_moves_8_4[9-this->no_start_moves_8_4]);
            this->no_start_moves_8_4--;
            this->best_move;
     }
    else {
         std::unordered_set<U16> special_moves; 

      if(b.data.player_to_play==BLACK){
         std::unordered_set<U16> special_moves_knight_b_1 = b.get_pseudolegal_moves_for_piece(b.data.b_knight_1);
         std::unordered_set<U16> special_moves_knight_b_2 = b.get_pseudolegal_moves_for_piece(b.data.b_knight_2);
         std::unordered_set<U16> special_moves_bishop_b = b.get_pseudolegal_moves_for_piece(b.data.b_bishop);
         special_moves.insert(special_moves_knight_b_1.begin(), special_moves_knight_b_1.end());
         special_moves.insert(special_moves_knight_b_2.begin(), special_moves_knight_b_2.end());
         special_moves.insert(special_moves_bishop_b.begin(),special_moves_bishop_b.end());
       }

     else if (b.data.player_to_play==WHITE){
         std::unordered_set<U16> special_moves_knight_w_1 = b.get_pseudolegal_moves_for_piece(b.data.w_knight_1);
         std::unordered_set<U16> special_moves_knight_w_2 = b.get_pseudolegal_moves_for_piece(b.data.w_knight_1);
         std::unordered_set<U16> special_moves_bishop_w = b.get_pseudolegal_moves_for_piece(b.data.w_bishop);
         special_moves.insert(special_moves_knight_w_1.begin(), special_moves_knight_w_1.end());
         special_moves.insert(special_moves_knight_w_2.begin(), special_moves_knight_w_2.end());
         special_moves.insert(special_moves_bishop_w.begin(),special_moves_bishop_w.end());
       }
        std::unordered_set<U16> last_moves=b.get_pseudolegal_moves_for_piece(this->last_moved);
        special_moves.insert( last_moves.begin(), last_moves.end());
    
        std::cout<<"2"<<std::endl;
        int depth_level=3; //4
        //if(!no_immidiate_threat(b)){
          //  depth_level=2;
        //}
     boardCounts[uniq_hash(board_to_str(&b.data))]++;

     auto moveset = b.get_legal_moves();
     std::cout <<"Legal Moves :";
     for (auto m : moveset) {
            std::cout << move_to_str(m) << " ";
        }
    std::cout <<"\n";
    double best_score = -99999;
    U16 best_move = random_sampling(moveset)[0];
    this->best_move=best_move;
    std::cout<<"random_move "<<move_to_str(this->best_move)<<"\n";
    std::chrono::milliseconds time_limit(20000);
    //Board* copy_board = b.copy();
    for(int depth=depth_level;depth<=depth_level;depth++){
        double best_max = -99999;
        U16 max_move=0;
        bool complete=true;
        std::cout<<"at depth "<<depth;
        for (auto move : moveset)
        {   if(special_moves.count(move)>0 && this->time_left>time_limit){
              depth = 4;
        }   if(this->time_left<time_limit){
              depth =2 ;
        }
            Board *copy_board = new Board(b);
            copy_board->do_move_(move);
          //  copy_board->data.player_to_play = (PlayerColor)(copy_board->data.player_to_play ^ (WHITE | BLACK));
            double score = alpha_beta_minimax(*copy_board, depth - 1, -99999, 99999, false, this,b.data.player_to_play);
          //  std::cout<<board_to_str(&b.data)<<" score "<<score<<"\n";
            outdata<<board_to_str(&b.data)<<" score "<<score<<" "<<depth<<" "<<move_to_str(move)<<"\n";

            if (score > best_max)
            {
                best_max = score;
                max_move = move;
            }

        }
           best_score=best_max;
           best_move=max_move;

        std::cout<<" best move "<<move_to_str(best_move)<<"\n";
      //  outdata<<" best move "<<move_to_str(best_move)<<"\n";
        this->best_move=best_move;
    }
   this->best_move = best_move;
        }
    return best_move;
}

U16 Engine::find_best_move_8_2(const Board &b){
     std::ofstream outdata; 
    outdata.open("log.txt", std::ios_base::app);
     std::unordered_set<U16> moveset = b.get_legal_moves();
     Board *b1 = new Board(b);
     if (moveset.size() == 0) {
        std::cout << "Could not get any moves from board!\n";
        std::cout << board_to_str(&b.data);
        this->best_move = 0;
    }
    bool is_legal_move=false;

    std::cout<<"1"<<std::endl;
    //outdata<<"1";
     if(this->no_start_moves_8_2!=0){
        if(b1->data.player_to_play==BLACK){
            U16 anti_move= get_anti_move(*b1,this->start_moves_8_2[8-this->no_start_moves_8_2]);
            outdata<<"Anti move"<< move_to_str(anti_move)<<std::endl;
            is_legal_move=moveset.count(anti_move)>0?true:false;
            b1->do_move_without_flip_(get_anti_move(*b1,this->start_moves_8_2[8-this->no_start_moves_8_2])); //correct this
        }
        else{
            is_legal_move=moveset.count(this->start_moves_8_2[8-this->no_start_moves_8_2])>0?true:false;
            b1->do_move_without_flip_(this->start_moves_8_2[8-this->no_start_moves_8_2]); 
        }
     }
     if(this->no_start_moves_8_2!=0 && b.data.player_to_play==WHITE && no_immidiate_threat(b) && no_immidiate_threat(*b1) && is_legal_move){
            this->best_move=this->start_moves_8_2[8-this->no_start_moves_8_2];
            this->no_start_moves_8_2--;
            return this->best_move;
        }
     else if(this->no_start_moves_8_2!=0 && b.data.player_to_play==BLACK && no_immidiate_threat(b) && no_immidiate_threat(*b1) && is_legal_move){
            this->best_move= get_anti_move(b,this->start_moves_8_2[8-this->no_start_moves_8_2]);
            this->no_start_moves_8_2--;
            this->best_move;
     }
    else {
          std::unordered_set<U16> special_moves; 

      if(b.data.player_to_play==BLACK){
         std::unordered_set<U16> special_moves_knight_b_1 = b.get_pseudolegal_moves_for_piece(b.data.b_knight_1);
         std::unordered_set<U16> special_moves_knight_b_2 = b.get_pseudolegal_moves_for_piece(b.data.b_knight_2);
         std::unordered_set<U16> special_moves_bishop_b = b.get_pseudolegal_moves_for_piece(b.data.b_bishop);
         special_moves.insert(special_moves_knight_b_1.begin(), special_moves_knight_b_1.end());
         special_moves.insert(special_moves_knight_b_2.begin(), special_moves_knight_b_2.end());
         special_moves.insert(special_moves_bishop_b.begin(),special_moves_bishop_b.end());
       }

     else if (b.data.player_to_play==WHITE){
         std::unordered_set<U16> special_moves_knight_w_1 = b.get_pseudolegal_moves_for_piece(b.data.w_knight_1);
         std::unordered_set<U16> special_moves_knight_w_2 = b.get_pseudolegal_moves_for_piece(b.data.w_knight_1);
         std::unordered_set<U16> special_moves_bishop_w = b.get_pseudolegal_moves_for_piece(b.data.w_bishop);
         special_moves.insert(special_moves_knight_w_1.begin(), special_moves_knight_w_1.end());
         special_moves.insert(special_moves_knight_w_2.begin(), special_moves_knight_w_2.end());
         special_moves.insert(special_moves_bishop_w.begin(),special_moves_bishop_w.end());
       }
        std::unordered_set<U16> last_moves=b.get_pseudolegal_moves_for_piece(this->last_moved);
        special_moves.insert( last_moves.begin(), last_moves.end());

        std::cout<<"2"<<std::endl;
        int depth_level=3; //4
        //if(!no_immidiate_threat(b)){
          //  depth_level=2;
        //}
     boardCounts[uniq_hash(board_to_str(&b.data))]++;

     auto moveset = b.get_legal_moves();
     std::cout <<"Legal Moves :";
     for (auto m : moveset) {
            std::cout << move_to_str(m) << " ";
        }
    std::cout <<"\n";
    double best_score = -99999;
    U16 best_move = random_sampling(moveset)[0];
    this->best_move=best_move;
    std::cout<<"random_move "<<move_to_str(this->best_move)<<"\n";
    std::chrono::milliseconds time_limit(20000);
    //Board* copy_board = b.copy();
    for(int depth=depth_level;depth<=depth_level;depth++){
        double best_max = -99999;
        U16 max_move=0;
        bool complete=true;
        std::cout<<"at depth "<<depth;
        for (auto move : moveset)
        {   if(special_moves.count(move)>0 && this->time_left>time_limit){
              depth = 4;
        }   if(this->time_left<time_limit){
              depth =2 ;
        }
            Board *copy_board = new Board(b);
            copy_board->do_move_(move);
          //  copy_board->data.player_to_play = (PlayerColor)(copy_board->data.player_to_play ^ (WHITE | BLACK));
            double score = alpha_beta_minimax(*copy_board, depth - 1, -99999, 99999, false, this,b.data.player_to_play);
          //  std::cout<<board_to_str(&b.data)<<" score "<<score<<"\n";
            outdata<<board_to_str(&b.data)<<" score "<<score<<" "<<depth<<" "<<move_to_str(move)<<"\n";

            if (score > best_max)
            {
                best_max = score;
                max_move = move;
            }

        }
           best_score=best_max;
           best_move=max_move;

        std::cout<<" best move "<<move_to_str(best_move)<<"\n";
      //  outdata<<" best move "<<move_to_str(best_move)<<"\n";
        this->best_move=best_move;
    }
   this->best_move = best_move;
        }
    return best_move;
}


void Engine::find_best_move(const Board& b) {
    auto start_time = std::chrono::high_resolution_clock::now();
    std::ofstream outdata; 
    outdata.open("log.txt", std::ios_base::app);

    if(!this->check_mate_moves.empty()){
        outdata<<this->check_mate_moves.top()<<" came from stack"<<std::endl;
        this->best_move=this->check_mate_moves.top();
        
        this->check_mate_moves.pop();
    }
    else{
    if(b.data.board_type==SEVEN_THREE){
        this->best_move=find_best_move_7_3(b);
        last_moved=getp1(this->best_move);
        outdata<<"move run "<<move_to_str(this->best_move) <<std::endl;
    }
    else if(b.data.board_type==EIGHT_FOUR){
        this->best_move=find_best_move_8_4(b);
        last_moved=getp1(this->best_move);
        outdata<<"move run "<<move_to_str(this->best_move) <<std::endl;
    }
    else if(b.data.board_type==EIGHT_TWO){
       this->best_move=find_best_move_8_2(b);
       last_moved=getp1(this->best_move);
       outdata<<"move run "<<move_to_str(this->best_move) <<std::endl;
    }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    outdata << " Time taken: " << duration.count() << " milliseconds" << std::endl;
  
    //std::this_thread::sleep_for(std::chrono::milliseconds(10000));

}

