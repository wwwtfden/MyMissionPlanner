#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QtWidgets>

class Waypoint{

protected:
    int num;
    int typep;
    int alttype;
    int pointtype;
    float param1;
    float param2;
    float param3;
    float param4;
    double lat;
    double lon;
    float alt;
    int automode;
    double dist;


public:
    Waypoint(){
        num = 0;
        typep = 0;
        alttype = 0;
        pointtype = 16;
        param1 = 0;
        param2 = 0;
        param3 = 0;
        param4 = 0;
        lat = 0;
        lon = 0;
        alt = 0;
        automode = 1;
        dist = 0;
    }
  //  void displaywpdata();
    void writewpdata();
    //void getwpdata();

    //функции записи переменных
    void write_num(int w_num);
    void write_typep(int w_typep);
    void write_alttype(int w_alttype);
    void write_pointtype(int w_pointtype);
    void write_param1(float w_param1);
    void write_param2(float w_param2);
    void write_param3(float w_param3);
    void write_param4(float w_param4);
    void write_lat(double w_lat);
    void write_lon(double w_lon);
    void write_alt(float w_alt);
    void write_automode(int w_automode);
    void write_dist(double w_dist);
//----------------------------------------------------
    int show_num();
    int show_typep();
    int show_alttype();
    int show_pointtype();
    float show_param1();
    float show_param2();
    float show_param3();
    float show_param4();
    double show_lat();
    double show_lon();
    float show_alt();
    int show_automode();
    double show_dist();

  //  int dist_btw_points(double lat1, double lon1, double lat2, double lon2);

};





#endif // WAYPOINT_H
