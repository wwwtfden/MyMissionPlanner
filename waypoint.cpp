#include "waypoint.h"
#include <cmath>
#include <QtMath>

//void getwpdata(){

//}


//--------------------------------------

void Waypoint::write_num(int w_num){
    num = w_num;
}

void Waypoint::write_typep(int w_typep){
    typep = w_typep;
}

void Waypoint::write_alttype(int w_alttype){
    alttype = w_alttype;
}

void Waypoint::write_pointtype(int w_pointtype){
    pointtype = w_pointtype;
}

void Waypoint::write_param1(float w_param1){
    param1 = w_param1;
}

void Waypoint::write_param2(float w_param2){
    param2 = w_param2;
}

void Waypoint::write_param3(float w_param3){
    param3 = w_param3;
}

void Waypoint::write_param4(float w_param4){
    param4 = w_param4;
}

void Waypoint::write_lat(double w_lat){
    lat = w_lat;
}

void Waypoint::write_lon(double w_lon){
    lon = w_lon;
}

void Waypoint::write_alt(float w_alt){
    alt = w_alt;
}

void Waypoint::write_automode(int w_automode){
    automode = w_automode;
}

void Waypoint::write_dist(double w_dist)
{
    dist = w_dist;
}

//---------------------------------------------------------


int Waypoint::show_num(){
    return num;
}

int Waypoint::show_typep(){
    return typep;
}

int Waypoint::show_alttype(){
    return alttype;
}

int Waypoint::show_pointtype(){
    return pointtype;
}

float Waypoint::show_param1(){
    return param1;
}

float Waypoint::show_param2(){
    return param2;
}

float Waypoint::show_param3(){
    return param3;
}

float Waypoint::show_param4(){
    return param4;
}

double Waypoint::show_lat(){
    return lat;
}

double Waypoint::show_lon(){
    return lon;
}

float Waypoint::show_alt(){
    return alt;
}

int Waypoint::show_automode(){
    return automode;
}
double Waypoint::show_dist(){
    return dist;
}


//---------------------------------------------------------
