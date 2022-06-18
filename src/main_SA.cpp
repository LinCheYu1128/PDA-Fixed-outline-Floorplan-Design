#include "header.h"
#include "structure.h"
using namespace std;
// map<string,NODE*> nodelist;
// map<string,PIN*> pinlist;
vector<BLOCK> nodelist;
vector<PIN> pinlist;
vector<NET> netlist;
int NumHardRectilinearBlocks;
int NumTerminals;
int NumNets;
int NumPins;
int Total_area;
double Outline_area;
double Outline;

void GetHardblocks(string file){
    ifstream inFile(file);
    string temp;
    getline(inFile, temp, ':');
    getline(inFile, temp, '\n');
    NumHardRectilinearBlocks = stoi(temp);
    getline(inFile, temp, ':');
    getline(inFile, temp, '\n');
    NumTerminals  = stoi(temp);
    cout <<"NumHardRectilinearBlocks="<< NumHardRectilinearBlocks<< endl;
    cout << "NumTerminals=" << NumTerminals << endl;
    getline(inFile, temp, '\n');
    Total_area = 0;
    for(int i=0; i<NumHardRectilinearBlocks; i++){
        BLOCK block;
        getline(inFile, temp, ' ');
        block.name = temp;
        getline(inFile, temp, ',');
        getline(inFile, temp, ',');
        getline(inFile, temp, ')');
        block.height = stoi(temp);
        getline(inFile, temp, '(');
        getline(inFile, temp, ',');
        block.width = stoi(temp);
        getline(inFile, temp, '\n');
        // cout << block->name << " "<< block->height <<" "<< block->width << endl;
        nodelist.push_back(block);
        block.area = block.height*block.width;
        Total_area += block.height*block.width;
        // nodelist.insert(pair<string,NODE*>(block->name,block));
    }
}

void GetPin(string file){
    ifstream inFile(file);
    string temp;
    for(int i=0; i<NumTerminals; i++){
        PIN pin;
        getline(inFile, temp, '\t');
        pin.name = temp;
        getline(inFile, temp, '\t');
        pin.coordinate.x = stoi(temp);
        getline(inFile, temp, '\n');
        pin.coordinate.y = stoi(temp);
        pinlist.push_back(pin);
        // pinlist.insert(pair<string,PIN*>(pin->name,pin));
    }
}

PIN FindPin(string name){
    for(const auto &item:pinlist){
        if(item.name==name) return item;
    }
}

BLOCK FindNode(string name){
    for(const auto &item:nodelist){
        if(item.name==name) return item;
    }
}

void GetNet(string file){
    ifstream inFile(file);
    string temp;
    int pininnet;
    getline(inFile, temp, ':');
    getline(inFile, temp, '\n');
    NumNets = stoi(temp);
    getline(inFile, temp, ':');
    getline(inFile, temp, '\n');
    NumPins = stoi(temp);
    for(int i=0; i<NumNets; i++){
        NET net ;
        getline(inFile, temp, ':');
        getline(inFile, temp, '\n');
        pininnet = stoi(temp);
        for(int j=0; j<pininnet; j++){
            getline(inFile, temp, '\n');
            if(temp[0]=='p'){
                PIN pin = FindPin(temp);
                // net->netarray_pin.push_back(pinlist.find(temp)->second);
                net.netarray_pin.push_back(pin);
            }
            else{
                BLOCK node = FindNode(temp);
                // net->netarray_node.push_back(nodelist.find(temp)->second);
                net.netarray_node.push_back(node);
            }
        }
        netlist.push_back(net);

    }
}

void WriteOutputFile(string file , BStarTree *tree){
    ofstream OutFile(file);
    OutFile << "Wirelength "<<tree->TotalWirelength<<endl;
    OutFile << "Blocks"<<endl;
    for(const auto &item : tree->tree_node){
        OutFile <<item.second->block.name<<' ';
        OutFile << item.second->coordinate.x <<' '<<item.second->coordinate.y<<' '<< item.second->rotate <<endl;
    }

}

bool SortPop(BStarTree const *tree_1, BStarTree const *tree_2){
    return tree_1->fitness < tree_2->fitness;
}

BStarTree* kTournamentSelection(vector<BStarTree*>population){
    BStarTree* parent;
    int index;
    index = rand()% population.size();
    parent = population.at(index);
    // vector<BStarTree*>::iterator index;
    for(int i=1; i<K_TOURNMENT; i++){
        index = rand()% population.size();
        if(parent->fitness > population.at(index)->fitness) parent = population.at(index);
    }
    return parent;
}

vector<BStarTree*> Crossover(vector<BStarTree*>population){
    vector<BStarTree*> offspring;
    offspring.clear();
    BStarTree* parent1 = kTournamentSelection(population);
    BStarTree* parent2 = kTournamentSelection(population);
    BStarTree* offspring1 ;
    BStarTree* offspring2 ;
    // cout << parent1->fitness << endl;
    double pci = (double) rand() / (RAND_MAX + 1.0);
    if(pci<CROSSOVER_RATE){
        offspring.push_back(parent1->leftSubtreeCrossover(parent2));
        offspring.push_back(parent2->leftSubtreeCrossover(parent1));
    }
    else{
        offspring1 = parent1->CopyTree(parent1);
        offspring2 = parent2->CopyTree(parent2);
        offspring1->Evaluate();
        offspring2->Evaluate();
        offspring.push_back(offspring1);
        offspring.push_back(offspring2);
    }
    return offspring;
}

vector<BStarTree*> Mutation(vector<BStarTree*>offspring){
    for(const auto &item : offspring){
        double pci = (double) rand() / (RAND_MAX + 1.0);
        if(pci<MUTAION_RATE){
            item->Perturbation();
        }
    }
    return offspring;
}

double CoolingSchedule(int iteration , double temperature, double delta) {
    int temp;
    if(temperature>=1000) temp = temperature*0.75;
    else if(temperature>=500) temp = temperature*0.8;
    else if(temperature>=100) temp = temperature*0.9;
    else temp = temperature* 0.99;
    return temp;
    // return temperature* lambda;
}

double AcceptProb(double temperature, double delta_cost) {
    double prob = exp(-(delta_cost)/temperature);
    return min(1.0,prob) ;
}

int main(int argc, char *argv[]){
    //*******************************************  Start time *****************************************//
    clock_t timeStart ;
	clock_t timeEnd ;
    int seed = (unsigned)time(0);
    srand(seed);
	timeStart = clock();
    //********************************************  Read File ****************************************//
    string Hardblocksfile = argv[1];
    string Netfile = argv[2];
    string Pinfile = argv[3];
    string Floorplanfile = argv[4];
    double dead_space_ratio = stod(argv[5]);
    
    GetHardblocks(Hardblocksfile);
    GetPin(Pinfile);
    GetNet(Netfile);
    Outline = sqrt(Total_area*(1+dead_space_ratio));
    Outline_area = Outline*Outline;
    // cout << "NumNets=" << NumNets << endl;
    // cout << "Outline = (" << Outline << ", "<< Outline << ")" <<endl; 
    //*****************************************   Initialize *****************************************//
    double average_cost = 0; 
    vector<BStarTree*> population;
    for(int i = 0; i < POPULATION; i++){
        BStarTree *tree = new BStarTree(nodelist);
        tree->Evaluate();
        average_cost += tree->fitness;
        population.push_back(tree);
    }
    average_cost = average_cost/POPULATION;
    cout << "average_cost" << average_cost << endl;
    sort(population.begin(), population.end(), SortPop);

    BStarTree *current_tree = population.at(0);
    BStarTree * best_tree = current_tree->CopyTree(current_tree);
    // best_tree->Evaluate();
    cout << "best fitness " << best_tree->fitness << endl;
    //******************************************* IN EACH GENERATION *********************************//
    BStarTree* nbr_tree;
    // double temperature =  average_cost/log(P);
    double temperature = 1000;
    int N = NumHardRectilinearBlocks*K;
    int MT, uphill, reject;
    double delta_avg_cost;
    int iter = 0;
    cout << "Initial temp = "<< temperature << endl;
    // for(int gen = 0; gen < GENERATION; gen++){
    while (1){
        uphill = 0;
        MT = 0;
        do{
            nbr_tree = current_tree->CopyTree(current_tree);
            nbr_tree->Perturbation();
            nbr_tree->Evaluate();

            if(nbr_tree->fitness<best_tree->fitness){
                delete best_tree;
                best_tree = nbr_tree->CopyTree(nbr_tree);
                // best_tree->Evaluate();
            }

            double delta_fit = nbr_tree->fitness - current_tree->fitness;
            
            double acpt_prob = AcceptProb(temperature, delta_fit);
            // if(NumHardRectilinearBlocks<=100) temperature = CoolingSchedule(iter, temperature, 0);
            
            double prob = (double) rand() / (RAND_MAX + 1.0);
            // cout << "delta "<< delta_fit<<" acpt_prob " <<acpt_prob << endl;
            /*** Uphill moving ***/
            if(prob <= acpt_prob){
                if(delta_fit>0) uphill++;
                delete current_tree;
                current_tree = nbr_tree->CopyTree(nbr_tree);
                // current_tree->Evaluate();
            }
            delete nbr_tree;
            
            iter++;
            MT++;
        }while (!(uphill > N || MT > 2 * N));
        
        cout <<"Iter "<< iter<< " time " << double(timeEnd - timeStart)/CLOCKS_PER_SEC 
        << " Outline: "<< Outline << " "<< best_tree->region.x <<" "<< best_tree->region.y << endl; 
        cout << "   temp "<< temperature <<"; cur fitness "<< current_tree->fitness 
        <<"; best fitness " << best_tree->fitness << endl;
        temperature = CoolingSchedule(iter, temperature, 0);
        // temperature = 500000/(iter);
        
        if(best_tree->region.x<Outline && best_tree->region.y<Outline) break;
        timeEnd = clock();
        if(double(timeEnd - timeStart)/CLOCKS_PER_SEC > 1140)break;
    }
    //********************************************* Output *******************************************//
    best_tree->Evaluate();
    best_tree->PrintTree();
    cout << "best fitness " << best_tree->fitness << endl;
    WriteOutputFile(Floorplanfile,best_tree);
    //*******************************************   End Time   ***************************************//
    timeEnd = clock();
    cout << "time= " << double(timeEnd - timeStart)/CLOCKS_PER_SEC << endl;
    cout << "seed =" << seed << endl;
    return 0;
}
// n100 best seed 1638589403
// ./hw3 ../testcases/n100.hardblocks ../testcases/n100.nets ../testcases/n100.pl ../output/n100.floorplan 0.1

/*
    while (1){
        MT = 0; 
        uphill = 0; 
        reject = 0;
        delta_avg_cost = 0;
        do{
            MT ++ ;
            nbr_tree = current_tree->CopyTree(current_tree);
            nbr_tree->Perturbation();
            nbr_tree->Evaluate();
            if(nbr_tree->fitness<best_tree->fitness){
                delete best_tree;
                best_tree = nbr_tree->CopyTree(nbr_tree);
                best_tree->Evaluate();
            }
            double delta_fit = nbr_tree->fitness - current_tree->fitness;
            double acpt_prob = AcceptProb(temperature, delta_fit);
            double prob = (double) rand() / (RAND_MAX + 1.0);
            if(prob <= acpt_prob){
                delete current_tree;
                current_tree = nbr_tree->CopyTree(nbr_tree);
                current_tree->Evaluate();
            }
            if(delta_fit > 0){
                uphill ++;
                delta_avg_cost += delta_fit;
            }
            delete nbr_tree;
        }while (!(uphill > N || MT > 2 * N));

        if(uphill > N) delta_avg_cost = delta_avg_cost/N;
        else delta_avg_cost = delta_avg_cost/(2*N);
        iter ++ ;
        temperature = CoolingSchedule(iter, temperature, delta_avg_cost);
        cout <<"Iter "<< iter << " Outline: "<< Outline << " "<< best_tree->region.x <<" "<< best_tree->region.y << endl; 
        cout << "   temp "<< temperature <<"; cur fitness "<< current_tree->fitness 
            <<"; best fitness " << best_tree->fitness << endl;
*/