/**
 * @file InspectorWidgetProcessor.h
 * @brief Class for extracting templates and detecting text over video files
 * @author Christian Frisson
 */

#ifndef InspectorWidgetProcessor_H
#define InspectorWidgetProcessor_H

#include <istream>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filewritestream.h"

#include <PartialCsvParser.hpp>
#include <vector>
#include <string>
#include <math.h>
//#include <algorithm>

#include <pugixml.hpp>

#include <deque>
#include <map>
#include <list>
#include <set>

#include "opencv2/core/version.hpp"
#include "opencv2/core/core.hpp"
#define CV_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define CV_VERSION_      CV_VERSION_CHECK(CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION)
#if CV_VERSION_ >= CV_VERSION_CHECK(3, 0, 0)
//#include "opencv2/imgcodecs.hpp"
#else
#define CAP_PROP_FRAME_WIDTH CV_CAP_PROP_FRAME_WIDTH 
#define CAP_PROP_FRAME_HEIGHT CV_CAP_PROP_FRAME_HEIGHT
#define CAP_PROP_FPS CV_CAP_PROP_FPS 
#define CAP_PROP_FRAME_COUNT CV_CAP_PROP_FRAME_COUNT 
#define CAP_PROP_POS_FRAMES CV_CAP_PROP_POS_FRAMES
#ifndef WINDOW_KEEPRATIO
#define WINDOW_KEEPRATIO 0x00000000
#endif
#endif
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "InspectorWidgetProcessorCommandParser.h"

////Methods:
////0: SQDIFF
////1: SQDIFF NORMED
////2: TM CCORR
////3: TM CCORR NORMED
////4: TM COEFF
////5: TM COEFF NORMED
//int match_method = 5; //for fast 5 (me) 3(web)

//// TM COEFF NORMED no thres color/gray
//// TM CCORR NORMED thres gray

//int max_Trackbar = 5;

//bool with_gui = false;
//const char* image_window = "Source Image";
//const char* result_window = "Result window";
//const char* text_window = "Detected text";

//float _threshold = 0.99;

std::string getStemName(std::string file_path);

std::string getExtension(std::string file_path);

void MatchingMethod( cv::Mat& srca,  // The reference image
                     cv::Mat& srcb,  // The template image
                     cv::Mat& dst,   // Template matching result
                     int match_method);

void fastMatchTemplate(cv::Mat& srca,  // The reference image
                       cv::Mat& srcb,  // The template image
                       cv::Mat& dst,   // Template matching result
                       int maxlevel,   // Number of levels
                       int match_method);

struct InspectorWidgetDate {
    int y;
    int m;
    int d;
    InspectorWidgetDate():y(-1),m(-1),d(-1){}
};

struct InspectorWidgetTime {
    int h;
    int m;
    float s;
    InspectorWidgetTime():h(-1),m(-1),s(-1){}
};

class InspectorWidgetProcessor{
    friend class InspectorWidgetProcessorCommandParser::operators;

public:
    InspectorWidgetProcessor();
    ~InspectorWidgetProcessor();
    //std::map<std::string, std::vector<float> > parseCSV(std::string file);
    std::map< std::string, std::map<std::string, std::vector<float> > > parseCSV(std::string file);

    std::string getStatusError();
    std::string getStatusSuccess();
    std::string getStatusPhase();
    float getStatusProgress();
    bool setStatusAndReturn(std::string phase, std::string error_message = "", std::string success_message = "" );

    float getElapsedSeconds(float timestamp);

    std::string getTemplateAnnotation(std::string name);

    std::string getAccessibilityAnnotation(std::string name);
    
    /// getAccessibilityUnderMouse
    /// time in sec
    /// x and y are ratios: pixel dimensions divided by video sizes
    std::vector<float> getAccessibilityUnderMouse(float time, float x, float y);

    std::vector<std::string> getTemplateList(){return template_list;}

    bool init( int argc, char** argv );
    bool init( std::vector<std::string> );
    void process();
    void abort();
    void clear();

    bool parseComputerVisionEvents(PCP::CsvConfig* cv_csv);
    bool parseClockTimestampsFile(std::string _path);
    bool parseFirstMinuteFrameFile(PCP::CsvConfig* fmf_csv);
    bool checkHookEvents(std::ifstream& hook_txt);
    bool parseHookEvents(std::ifstream& hook_txt, bool using_clocktime);
    bool parseFilterings( std::vector<std::string>& filtering_list);
    bool applyFilterings();

    ///

    virtual bool s2b(std::string _string){
        if(_string == "True"){
            return true;
        }
        else if(_string == "False"){
            return false;
        }

        std::map<std::string, std::vector<float> >::iterator _log_val = log_val.find( _string);
        if(_log_val != log_val.end()){
            bool applies = (_log_val->second[current_frame] > _threshold);
            return applies;
        }

        std::map<std::string, std::vector<std::string> >::iterator _log_txt = log_txt.find( _string);
        if(_log_txt != log_txt.end()){
            bool applies = (_log_txt->second[current_frame] != " ");
            return applies;
        }
        return false;
    }

    virtual std::string b2s(bool _bool){
        if(_bool){
            return "True";
        }
        else{
            return "False";
        }
    }


protected:
    std::string status_success;
    std::string status_error;
    std::string status_phase;
    float status_progress;
    bool active;

    void matchTemplates();
    bool detectText();

    cv::Mat img;
    cv::Mat dst;
    cv::Mat ref_gray, tpl_gray;
    std::map<std::string,cv::Mat> templates;
    std::map<std::string,cv::Mat> gray_templates;
    std::map<std::string, float > template_x,template_y,template_val;
    std::map<std::string, std::vector<float> > template_vals;
    std::map<std::string, int> template_w,template_h;
    std::map<std::string, int> logged_template_w,logged_template_h;

    std::map<std::string, float > text_x,text_y;
    std::map<std::string, std::string > text_txt;

    std::ofstream file;

    std::map<std::string,std::ofstream> csvfile;

    std::string hook_path;
    int hook_header_size;

    std::string datapath;
    std::string videostem;
    cv::VideoCapture cap;

    int frame;
    int csv_frame;

    int video_w;
    int video_h;
    int video_frames;

    int current_frame;

    int start_h;
    int start_m;
    int first_minute_frames;

    float fps;

    bool parse_full_video;

    std::map<std::string, std::vector<float> > log_val;
    std::map<std::string, std::vector<std::string> > log_txt;
    std::map<std::string, std::vector<float> > log_x;
    std::map<std::string, std::vector<float> > log_y;

    InspectorWidgetTime start_t,end_t;
    InspectorWidgetDate start_d;
    uint64 start_clock;
    uint64 end_clock;

    std::map<int, uint64> ts_time;
    std::map<int, uint64> ts_clock;

    std::vector<std::string> supported_extraction_tests;
    std::vector<std::string> supported_extraction_actions;
    std::vector<std::string> supported_conversion_tests;
    std::vector<std::string> supported_conversion_actions;
    std::vector<std::string> supported_accessibility_tests;
    std::vector<std::string> supported_accessibility_actions;
    std::vector<std::string> supported_input_hook_tests;
    std::vector<std::string> supported_input_hook_actions;

    std::list<std::string> template_matching_logged_dep_list;
    std::map<std::string,std::vector<std::string> > template_matching_logged_dep_map;

    std::list<std::string> template_matching_new_dep_list;
    std::map<std::string,std::vector<std::string> > template_matching_new_dep_map;

    std::map<std::string,std::vector<std::string> > template_matching_dep_map;

    std::list<std::string> text_detect_logged_dep_list;
    std::map<std::string,std::vector<std::string> > text_detect_logged_dep_map;

    std::list<std::string> text_detect_new_dep_list;
    std::map<std::string,std::vector<std::string> > text_detect_new_dep_map;

    std::map<std::string,std::vector<std::string> > text_detect_dep_map;

    std::vector<std::string> template_list;

    std::set<std::string> text_detect_list;

    std::map<std::string,std::string > template_matching_type;
    std::map<std::string,std::string > template_matching_test;

    std::map<std::string,std::string > text_detect_type;
    std::map<std::string,std::string > text_detect_test;

    std::map<std::string,std::vector<int> > inrect_map;

    std::vector<std::string> filters;
    std::map<std::string,std::string> filtering_test;
    std::map<std::string,std::vector<std::string> > filtering_deps;
    std::map<std::string,std::string> filtering_action;
    std::map<std::string,std::vector<std::string> > filtering_variables;

    std::vector<std::string> inputhooks;
    std::map<std::string,std::string> inputhook_test;
    std::map<std::string,std::vector<std::string> > inputhook_deps;
    std::map<std::string,std::string> inputhook_action;
    std::map<std::string,std::vector<std::string> > inputhook_variables;

    int match_method;
    int max_Trackbar;
    bool with_gui;
    const char* image_window;
    const char* result_window;
    const char* text_window;
    float _threshold;

    InspectorWidgetProcessorCommandParser::stacks parser_stacks;
    InspectorWidgetProcessorCommandParser::operators* parser_operators;

};

#endif //InspectorWidgetProcessor_H
