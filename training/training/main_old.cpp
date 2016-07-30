#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <vector>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <stdlib.h>

using namespace std;

#define FEATURE_DIR "../features/"
#define PICTURE_DIR "../pictures/"

struct wrapper{
    char *name;
    uint64_t mac;
    uint16_t id;
};

struct valid_entries{
    char *name_image;
    char *name_features;
    uint64_t mac;
    uint16_t id;
};

int add_entry(vector<struct wrapper> *list, char *str, const char *dir, const char *match);
int combine_lists(vector<struct wrapper> *features, vector<struct wrapper> *pictures, vector<struct valid_entries> *list);

int main() {
    
    vector<struct wrapper> features;
    vector<struct wrapper> pictures;
    vector<struct valid_entries> list;
    const char * dir_features = FEATURE_DIR;
    const char * dir_pictures = PICTURE_DIR;
    const char * match_features = "%lx_%hd.feature\n";
    const char * match_pictures = "%lx_%hd.png\n";

    DIR *dp;
    struct dirent *ep;
    dp = opendir(dir_features);
    if (dp != NULL) {
        while (ep = readdir(dp))
            add_entry(&features, ep->d_name, dir_features, match_features);
        (void) closedir(dp);
    } else
        perror("Couldn't open the directory features");
    
    dp = opendir(dir_pictures);
    if (dp != NULL) {
        while (ep = readdir(dp))
            add_entry(&pictures, ep->d_name, dir_pictures, match_pictures);
        (void) closedir(dp);
    } else
        perror("Couldn't open the directory pictures");
    
    
    combine_lists(&features, &pictures, &list);
    
    /*for(int i = 0; i < list.size(); i++){
        struct valid_entries *item = &(list.at(i));
        printf("valid entry:: %s %s %lx %d\n", item->name_features, item->name_image, item->mac, item->id);
    }*/
    
    //read out files from valid list
    //loop for classify to recognize pictures
    //save result in big file with feature vector and class
    //train classifier on this data and save trained classifier in file classifier.xml
    
    
    return 0;
}

int add_entry(vector<struct wrapper> *list, char *str, const char *dir, const char *match) {
    struct wrapper * item = (struct wrapper *) malloc(sizeof (struct wrapper));
    sscanf(str, match, &item->mac, &item->id);
    if (item->mac != 0) {
        size_t string_size = strlen(str) + strlen(dir);
        char * name = (char *) malloc(string_size);
        strcpy(name, dir);
        strcpy(name + strlen(dir), str);
        item->name = name;
        //printf("%s %lx %d\n",item->name, item->mac, item->id);
        list->push_back(*item);
        return 0;
    }
    return -1;
}


int combine_lists(vector<struct wrapper> *features, vector<struct wrapper> *pictures, vector<struct valid_entries> *list){
    while(!features->empty()){
        struct wrapper *feat_item = &features->back();
        for(int i = 0; i < pictures->size(); i++){
            struct wrapper *pic_item = &(pictures->at(i));
            if(feat_item->mac == pic_item->mac && feat_item->id == pic_item->id){
                struct valid_entries *item = (struct valid_entries *)malloc(sizeof(struct valid_entries));
                item->name_image = pic_item->name;
                item->name_features = feat_item->name;
                item->mac = feat_item->mac;
                item->id = feat_item->id;
                list->push_back(*item);
                features->pop_back();
                pictures->erase(pictures->begin() + i);
                i = pictures->size();
            }
        }
    }
}
