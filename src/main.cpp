#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <utility>
#include <algorithm>
using namespace std;
#define A 0
#define B 1
#define NO 0
#define YES 1
#define UNLOCKED 0
#define LOCKED 1


struct CELL {
	int id;
	int gain;
	int size;
	int pin;
	char region;
	char lock;
	vector<int> nets_in_cell;
	CELL* prev;
	CELL* next;
};

struct NET {
	int id;
	vector<int> cells_in_A;/****bear said : no need ****/
	vector<int> cells_in_B;
	int cell_num_A;
	int cell_num_B;
};


unordered_map<int, CELL*> cells;
unordered_map<int, NET*> nets;
vector<CELL*> Bucket_list;
vector<int>Bucket_list_num;
int Region_A = 0, Region_B = 0;
int MaxGain, NowGain, MaxPin;
int total_cut_size;

void UpdateBucket(CELL* cell, int old_gain, int new_gain){
	if(cell->next != NULL) cell->next->prev = cell->prev;
    if(cell->prev != NULL) cell->prev->next = cell->next;
    if(Bucket_list[old_gain+MaxPin] == cell) Bucket_list[old_gain+MaxPin] = cell->next;
	cell->next = NULL;
	cell->prev = NULL;
	--Bucket_list_num[old_gain+MaxPin];

	if(old_gain+MaxPin == MaxGain){
		while(Bucket_list_num[MaxGain]==0 && MaxGain > 0){
			--MaxGain; /**************************Caution : need a barrier**************************/
		}
	}

	if(Bucket_list[new_gain+MaxPin] == NULL)
		Bucket_list[new_gain+MaxPin] = cell;
	else{
		Bucket_list[new_gain+MaxPin]->prev = cell;
		cell->next = Bucket_list[new_gain+MaxPin];
		Bucket_list[new_gain+MaxPin] = cell;
	}
	++Bucket_list_num[new_gain+MaxPin];
	if(new_gain+MaxPin > MaxGain) MaxGain = new_gain+MaxPin;
	cell->gain = new_gain;
}

void MoveToB(CELL* cell){
	int net_id;
	NET* net_tmp;
    cell->next = NULL;
	cell->prev = NULL;
	cell->lock = 1;

	for (auto it = cell->nets_in_cell.begin() ; it != cell->nets_in_cell.end(); ++it){
			// net_tmp is net pointer pointing to nets in cell.
			net_id = (*it); // catch one net pointer;
			net_tmp = nets[net_id];

			if(net_tmp->cell_num_B == 0) {
				for(auto iter = net_tmp->cells_in_A.begin() ; iter != net_tmp->cells_in_A.end(); ++iter){
					if(!cells[*iter]->lock){
						UpdateBucket(cells[*iter], cells[*iter]->gain, cells[*iter]->gain+1);
					}
				}
			}else if((net_tmp->cell_num_B == 1) && !cells[net_tmp->cells_in_B.back()]->lock){
				UpdateBucket(cells[net_tmp->cells_in_B.back()], cells[net_tmp->cells_in_B.back()]->gain, cells[net_tmp->cells_in_B.back()]->gain-1);
			}

			/*implement move*/ /*****************caution: change of size***********************/
			cell->region = B;
			++net_tmp->cell_num_B;
			net_tmp->cells_in_B.push_back(cell->id);

			--net_tmp->cell_num_A;
			net_tmp->cells_in_A.erase(std::find(net_tmp->cells_in_A.begin(), net_tmp->cells_in_A.end(), cell->id));


			if(net_tmp->cell_num_A == 0){
				for(auto iter = net_tmp->cells_in_B.begin() ; iter != net_tmp->cells_in_B.end(); ++iter){
					if(!cells[*iter]->lock)
						UpdateBucket(cells[*iter], cells[*iter]->gain, cells[*iter]->gain-1);
				}
			}else if(net_tmp->cell_num_A == 1 && !cells[net_tmp->cells_in_A.back()]->lock)
					UpdateBucket(cells[net_tmp->cells_in_A.back()], cells[net_tmp->cells_in_A.back()]->gain, cells[net_tmp->cells_in_A.back()]->gain+1);
	}

}


void MoveToA(CELL* cell){
	int net_id;
	NET* net_tmp;
    cell->next = NULL;
	cell->prev = NULL;
	cell->lock = 1;
	for (auto it = cell->nets_in_cell.begin() ; it != cell->nets_in_cell.end(); ++it){
			// net_tmp is net pointer pointing to nets in cell.
			net_id = (*it); // catch one net pointer;
			net_tmp = nets[net_id];
			if(net_tmp->cell_num_A == 0) {
				for(auto iter = net_tmp->cells_in_B.begin() ; iter != net_tmp->cells_in_B.end(); ++iter){
					if(!cells[*iter]->lock){
						UpdateBucket(cells[*iter], cells[*iter]->gain, cells[*iter]->gain+1);
					}
				}
			}else if(net_tmp->cell_num_A == 1 && !cells[net_tmp->cells_in_A.back()]->lock){
				UpdateBucket(cells[net_tmp->cells_in_A.back()], cells[net_tmp->cells_in_A.back()]->gain, cells[net_tmp->cells_in_A.back()]->gain-1);
			}

			/*implement move*/ /*****************caution: change of size***********************/
			cell->region = A;
			++net_tmp->cell_num_A;
			net_tmp->cells_in_A.push_back(cell->id);

			--net_tmp->cell_num_B;
			net_tmp->cells_in_B.erase(find(net_tmp->cells_in_B.begin(), net_tmp->cells_in_B.end(), cell->id));

			if(net_tmp->cell_num_B == 0){
				for(auto iter = net_tmp->cells_in_A.begin() ; iter != net_tmp->cells_in_A.end(); ++iter){
					if(!cells[*iter]->lock)
						UpdateBucket(cells[*iter], cells[*iter]->gain, cells[*iter]->gain-1);
				}
			}else if(net_tmp->cell_num_B == 1 && !cells[net_tmp->cells_in_B.back()]->lock)
					UpdateBucket(cells[net_tmp->cells_in_B.back()], cells[net_tmp->cells_in_B.back()]->gain, cells[net_tmp->cells_in_B.back()]->gain+1);
	}

}

int main(int argc, char const *argv[])
{

    double START,END,IOSTART,IOEND,IOTIME=0;
    START = clock();
    /*****buildup Output*********/
    vector<string> rowA;
    vector<string> rowB;

    stringstream ss;
    IOSTART = clock();
    fstream fs(argv[1], fstream::in);
	  char c;
    int cid, csize, nid;
	  while(fs >> c){
		fs >> cid >> csize;
    CELL* cell = new CELL();
		cell->id = cid;
		cell->gain = 0;
		cell->size = csize;
		cell->pin = 0;
		cell->lock = 0;
		cell->prev = NULL;
		cell->next = NULL;
		if(Region_A < Region_B){
			(*cell).region = A;
			Region_A+= csize;
		}else{
			(*cell).region = B;
			Region_B+= csize;
		}
		cells.insert(std::make_pair(cid, cell));
    }
	  fs.close();
      
	  fs.open(argv[2], fstream::in);
	  string str;
    CELL* cell_tmp;
	  set<int> filter;
	  while(fs >> str){
  		fs>>c>>nid>>c;
  		fs >> c;
  		NET* net = new NET();
  		net->id = nid;
  		net->cell_num_A = 0;
  		net->cell_num_B = 0;
  		filter.clear();
		  while(c == 'c'){
  			fs >> cid;
  			//cell_tmp is a pointer pointing to each cell in net;
  			if(filter.find(cid)==filter.end()){
  				cell_tmp = cells[cid];
  				(*cell_tmp).nets_in_cell.push_back(nid);
  				if((*cell_tmp).region == A){
  					(*net).cells_in_A.push_back(cid);
  					++(*net).cell_num_A;
  				}else{
  					(*net).cells_in_B.push_back(cid);
  					++(*net).cell_num_B;
  				}
  				++(*cell_tmp).pin;
  				filter.insert(cid);
  			}
  			fs >> c;
		}




		if(net->cell_num_A == 1 && net->cell_num_B > 0)  ++ cells[net->cells_in_A.back()]->gain;
		if(net->cell_num_B == 1 && net->cell_num_A > 0)  ++ cells[net->cells_in_B.back()]->gain;
		if(net->cell_num_A == 0){
			for (vector<int>::iterator it = net->cells_in_B.begin() ; it != net->cells_in_B.end(); ++it)
				--cells[(*it)]->gain;
		}
		if(net->cell_num_B == 0){
			for (vector<int>::iterator it = net->cells_in_A.begin() ; it != net->cells_in_A.end(); ++it)
				--cells[(*it)]->gain;
		}
		if(net->cell_num_B && net->cell_num_A){++total_cut_size;}
		nets.insert (std::make_pair(nid, net));

	}fs.close();

	IOEND = clock();
    IOTIME+= IOEND-IOSTART;
	/*set up Bucket List*/
	MaxPin = 0;
	for(auto it = cells.begin(); it != cells.end(); ++it){
		if( it->second->pin > MaxPin ) MaxPin = it->second->pin;
	}

	Bucket_list.assign(2*MaxPin+1,NULL);
	Bucket_list_num.assign(2*MaxPin+1,0);
	MaxGain = MaxPin;
	for ( auto it = cells.begin(); it != cells.end(); ++it ){
		if(Bucket_list[it->second->gain + MaxPin] == NULL)
			Bucket_list[it->second->gain + MaxPin] = it->second;
		else{
			Bucket_list[it->second->gain + MaxPin]->prev = it->second;
			it->second->next = Bucket_list[it->second->gain + MaxPin];
			Bucket_list[it->second->gain + MaxPin] = it->second;
		}
		++Bucket_list_num[it->second->gain + MaxPin];
		if(it->second->gain+MaxPin > MaxGain) MaxGain = it->second->gain + MaxPin;
	}


	/*Start Moving the cell*/
	NowGain = MaxGain; /* if NowGain == 0(MaxPin)??*/

	vector<CELL*> changed_cells;
	int MaxPartial, CurrentPartial;
	int move_step, target_step;

	int time = 0;
	while(time < 15){
		rowA.clear();
        rowB.clear();
		MaxPartial = 0;
		CurrentPartial = 0;
		move_step = 0;
		target_step = 0;
		changed_cells.clear();
			while(MaxGain  >= MaxPin - (MaxPin/2)){
				int NowGain = MaxGain;
				CELL* ctmp = Bucket_list[NowGain];
				while(ctmp!=NULL && NowGain >= MaxPin - (MaxPin/2)){

                    //cout << "select c" << ctmp->id << ": gain = " << ctmp->gain << ", region = "<< ctmp->region << endl;


					if(ctmp->region == A){
						Region_A-= ctmp->size;
						Region_B+= ctmp->size;
						if(abs(Region_A - Region_B) < (Region_A + Region_B)/10){
							if(ctmp->next != NULL) ctmp->next->prev = ctmp->prev;
							if(ctmp->prev != NULL) ctmp->prev->next = ctmp->next;
                            if(Bucket_list[NowGain] == ctmp) Bucket_list[NowGain] = ctmp->next;

                            //cout << "Move to B"<< endl;
							MoveToB(ctmp);
							changed_cells.push_back(ctmp);
							CurrentPartial+=(*ctmp).gain;
							if(CurrentPartial > MaxPartial){
								MaxPartial = CurrentPartial;
								target_step = move_step;
							}++move_step;

							--Bucket_list_num[NowGain];
							if(NowGain == MaxGain){
								while(Bucket_list_num[MaxGain]==0 && MaxGain > 0)
									--MaxGain; /**************************Caution : need a barrier**************************/
							}

							break;
						}else{
							Region_A+= ctmp->size;
							Region_B-= ctmp->size;
							//cout << "Fail to move"<< endl;
							if((*ctmp).next!=NULL){ctmp = (*ctmp).next;}
							else{
								NowGain--;
								while(NowGain > 0 && Bucket_list[NowGain] == NULL){
									NowGain--;
                                }
								ctmp = Bucket_list[NowGain];
							}
						}
					}else{
						Region_A+= ctmp->size;
						Region_B-= ctmp->size;
						if(abs(Region_A - Region_B) < (Region_A + Region_B)/10){
							if(ctmp->next != NULL) ctmp->next->prev = ctmp->prev;
							if(ctmp->prev != NULL) ctmp->prev->next = ctmp->next;
                            if(Bucket_list[NowGain] == ctmp) Bucket_list[NowGain] = ctmp->next;
                           // cout << "Move to A"<< endl;
							MoveToA(ctmp);
							changed_cells.push_back(ctmp);
							CurrentPartial+=(*ctmp).gain;
							if(CurrentPartial > MaxPartial){
								MaxPartial = CurrentPartial;
								target_step = move_step;
							}++move_step;

							--Bucket_list_num[NowGain];
							if(NowGain == MaxGain){
								while(Bucket_list_num[MaxGain]==0 && MaxGain > 0)
									--MaxGain; /**************************Caution : need a barrier**************************/
							}

							break;
						}else{
							Region_A-= ctmp->size;
							Region_B+= ctmp->size;
							//cout << "Fail to move"<< endl;
							if((*ctmp).next!=NULL){ctmp = (*ctmp).next;}
							else{
								NowGain--;
								while(NowGain > 0 && Bucket_list[NowGain] == NULL){
									NowGain--;
                                }
								ctmp = Bucket_list[NowGain];
							}
						}
					}
				}
				if(ctmp == NULL || NowGain < MaxPin - (MaxPin/2)) break;


			}


		/*****move wanted cells******/
		for (int i=target_step+1; i<move_step; i++){
			if(changed_cells[i]->region == A)
				changed_cells[i]->region = B;
			else	changed_cells[i]->region = A;
		}
		/****reset nets*******/
		for(auto it = nets.begin(); it != nets.end(); ++it){
			it->second->cell_num_A = 0;
			it->second->cell_num_B = 0;
			it->second->cells_in_A.clear();
			it->second->cells_in_B.clear();
		}


		/*****reset cells lock and gain*******/
		Region_A = 0; Region_B = 0;
		for(auto it = cells.begin(); it != cells.end(); ++it){
			ss.str("");
			it->second->gain = 0;
			it->second->lock = 0;
			it->second->prev = NULL;
			it->second->next = NULL;
			if(it->second->region == A){
				for (vector<int>::iterator iter = it->second->nets_in_cell.begin() ; iter != it->second->nets_in_cell.end(); ++iter){
					++nets[(*iter)]->cell_num_A;
					nets[(*iter)]->cells_in_A.push_back(it->second->id);
				}
				Region_A+= it->second->size;
				ss<<"c"<<it->second->id<<endl;
				rowA.push_back(ss.str());
			}else{
				for (vector<int>::iterator iter = it->second->nets_in_cell.begin() ; iter != it->second->nets_in_cell.end(); ++iter){
					++nets[(*iter)]->cell_num_B;
					nets[(*iter)]->cells_in_B.push_back(it->second->id);
				}
				Region_B+= it->second->size;
				ss<<"c"<<it->second->id<<endl;
				rowB.push_back(ss.str());
			}
		}
		total_cut_size = 0;
		for(auto it = nets.begin(); it != nets.end(); ++it){
			if(it->second->cell_num_A == 1 && it->second->cell_num_B > 0)  ++ cells[it->second->cells_in_A.back()]->gain;
			if(it->second->cell_num_B == 1 && it->second->cell_num_A > 0)  ++ cells[it->second->cells_in_B.back()]->gain;
			if(it->second->cell_num_A == 0){
				for (vector<int>::iterator iter = it->second->cells_in_B.begin() ; iter != it->second->cells_in_B.end(); ++iter)
					--cells[(*iter)]->gain;
			}
			if(it->second->cell_num_B == 0){
				for (vector<int>::iterator iter = it->second->cells_in_A.begin() ; iter != it->second->cells_in_A.end(); ++iter)
					--cells[(*iter)]->gain;
			}
			if(it->second->cell_num_A && it->second->cell_num_B) ++total_cut_size;
		}

		/***SetUp Bucket_list ***/
		Bucket_list.clear();
		Bucket_list_num.clear();
		Bucket_list.assign(2*MaxPin+1,NULL);
		std::fill(Bucket_list_num.begin(), Bucket_list_num.end(), 0);
		MaxGain = MaxPin;
		for ( auto it = cells.begin(); it != cells.end(); ++it ){
			if(Bucket_list[it->second->gain + MaxPin] == NULL)
				Bucket_list[it->second->gain + MaxPin] = it->second;
			else{
				Bucket_list[it->second->gain + MaxPin]->prev = it->second;
				it->second->next = Bucket_list[it->second->gain + MaxPin];
				Bucket_list[it->second->gain + MaxPin] = it->second;
			}
			++Bucket_list_num[it->second->gain + MaxPin];
			if((it->second->gain + MaxPin) > MaxGain) MaxGain = it->second->gain + MaxPin;
		}

		++time;


	}

    IOSTART = clock();
	/*****output block*****/
	  std::ofstream ofs (argv[3], std::ofstream::out);
  
    
    ofs << "cut_size " << total_cut_size << endl;
	  ofs << "A " << Region_A << endl;
	  for(auto it = rowA.begin(); it != rowA.end(); ++it)
        ofs << *it;
	  ofs << "B " << Region_B << endl;;
    for(auto it = rowB.begin(); it != rowB.end(); ++it)
        ofs << *it;
    ofs.close();
    
    IOEND = clock();
    IOTIME+=IOEND-IOSTART;
    
    END = clock();
    cout << "I/O Time : " << IOTIME/(double)(CLOCKS_PER_SEC) << endl;
    cout << "Computation Time : " << (END-START-IOTIME)/(double)(CLOCKS_PER_SEC) << endl;
}
