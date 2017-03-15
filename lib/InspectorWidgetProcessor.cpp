/**
 * @file InspectorWidgetProcessor.cpp
 * @brief Class for extracting templates and detecting text over video files
 * @author Christian Frisson
 */

#include <tesseract/baseapi.h>

#include "InspectorWidgetProcessor.h"

#include <uiohook.h>

using namespace rapidjson;
using namespace std;
using namespace cv;

std::string modifierString(int keycodeint){
    std::string  modifier_string = "";
    switch(keycodeint){
    // Begin Modifier and Control Keys
    case VC_SHIFT_L: //0x002A
        modifier_string = "Shift (Left)";
        break;
    case VC_SHIFT_R: //0x0036
        modifier_string = "Shift (Right)";
        break;
    case VC_CONTROL_L: //0x001D
        modifier_string = "Ctrl (Left)";
        break;
    case VC_CONTROL_R: //0x0E1D
        modifier_string = "Ctrl (Right)";
        break;
    case VC_ALT_L: //0x0038	// Option or Alt Key
        modifier_string = "Alt (Left)";
        break;
    case VC_ALT_R: //0x0E38	// Option or Alt Key
        modifier_string = "Alt (Right)";
        break;
    case VC_META_L: //0x0E5B	// Windows or Command Key
        modifier_string = "Meta (Left)";
        break;
    case VC_META_R: //0x0E5C	// Windows or Command Key
        modifier_string = "Meta (Right)";
        break;
    case VC_CONTEXT_MENU: //0x0E5D
        modifier_string = "Context Menu";
        break;
        // End Modifier and Control Keys
    default:
        modifier_string = "";
        break;
    }
    return modifier_string;
}

bool canformword(int keycodeint){
    bool _canformword = false;
    switch(keycodeint){
    // Begin Alphanumeric Zone
    case VC_1: //0x0002
    case VC_2: //0x0003
    case VC_3: //0x0004
    case VC_4: //0x0005
    case VC_5: //0x0006
    case VC_6: //0x0007
    case VC_7: //0x0008
    case VC_8: //0x0009
    case VC_9: //0x000A
    case VC_0: //0x000B

    case VC_MINUS: //0x000C	// '-'

    case VC_A: //0x001E
    case VC_B: //0x0030
    case VC_C: //0x002E
    case VC_D: //0x0020
    case VC_E: //0x0012
    case VC_F: //0x0021
    case VC_G: //0x0022
    case VC_H: //0x0023
    case VC_I: //0x0017
    case VC_J: //0x0024
    case VC_K: //0x0025
    case VC_L: //0x0026
    case VC_M: //0x0032
    case VC_N: //0x0031
    case VC_O: //0x0018
    case VC_P: //0x0019
    case VC_Q: //0x0010
    case VC_R: //0x0013
    case VC_S: //0x001F
    case VC_T: //0x0014
    case VC_U: //0x0016
    case VC_V: //0x002F
    case VC_W: //0x0011
    case VC_X: //0x002D
    case VC_Y: //0x0015
    case VC_Z: //0x002C

        _canformword = true;
        break;

    default:
        _canformword = false;
        break;

    }
    return _canformword;
}

bool canformword(std::string keychar){
    bool _canformword = false;
    if(keychar.size() != 1){
        return _canformword;
    }
    int _keychar = (char)keychar[0];
    if( _keychar == 45 // '-'
            || (_keychar >= 48 && _keychar <= 57) // 0-9
            || (_keychar >= 64 && _keychar <= 90) // @ A-Z
            || (_keychar >= 97 && _keychar <= 122) // @ a-z
            )
    {
        _canformword = true;
    }

    return _canformword;
}

vector<string> splitconstraint(const string& s, char sep) {
    string::size_type seppos = s.find(sep);
    string::size_type prevseppos = 0;
    vector<string> result;

    if (seppos != string::npos) {
        result.push_back(s.substr(0, seppos));
    }
    else {
        result.push_back(s);
        return result;
    }

    if (seppos+1 == string::npos) {
        return result;
    }

    prevseppos = seppos;
    seppos = s.find(sep, prevseppos+1);

    while (seppos != string::npos) {
        if (prevseppos+1 == string::npos) return result;

        result.push_back( s.substr(prevseppos + 1, seppos - (prevseppos + 1)) );
        prevseppos = seppos;
        seppos = s.find(sep, prevseppos+1);
    }

    // add last bit
    result.push_back( s.substr(prevseppos+1) );

    return result;
}


std::string time2tc(int h, int m, float s){
    std::stringstream tc;
    if (h>10)
        tc << h;
    else if(h==0)
        tc << "00";
    else
        tc << "0" << h;
    tc << ":";

    if (m>10)
        tc << m;
    else if(m==0)
        tc << "00";
    else
        tc << "0" << m;
    tc << ":";

    tc.precision(4);
    tc << fixed;

    if (s>10)
        tc << s;
    else
        tc << "0" << s;

    return tc.str();
}

std::string date2dc(int y, int m, int d){
    std::stringstream dc;
    dc << y;
    dc << "-";

    if (m>=10)
        dc << m;
    else if(m==0)
        dc << "00";
    else
        dc << "0" << m;
    dc << "-";

    if (d>=10)
        dc << d;
    else if(d==0)
        dc << "00";
    else
        dc << "0" << d;

    return dc.str();
}

std::string frames2tc(float frames, float fps){
    float time = frames/(float)fps;
    int h = floor(time/3600.0);
    int m = floor( (time - h*3600.0)/60.0 );
    float s = time - h*3600.0 - m*60.0;
    return time2tc(h,m,s);
}

std::string frames2dtc(float frames, float fps, InspectorWidgetDate start_date, InspectorWidgetTime start_time){
    float time = frames/(float)fps + start_time.h*3600 + start_time.m*60;
    int h = floor(time/3600.0);
    int m = floor( (time - h*3600.0)/60.0 );
    float s = time - h*3600.0 - m*60.0;
    return date2dc(start_date.y, start_date.m, start_date.d) + "-" + time2tc(h,m,s);
}

void header(PrettyWriter<StringBuffer>& writer){
    writer.StartObject();
    writer.String("localisation");
    writer.StartArray();
    writer.StartObject();
    writer.String("sublocalisations");
    writer.StartObject();
    writer.String("localisation");
    writer.StartArray();
}

void event(PrettyWriter<StringBuffer>& writer, float in, float fps, std::string label){

    writer.StartObject();
    writer.String("label");
    writer.String(label.c_str());
    writer.String("tc");
    writer.String(frames2tc(in,fps).c_str());
    writer.String("tclevel");
    writer.Double(1.0);
    writer.EndObject();
}

void segment(PrettyWriter<StringBuffer>& writer, float in, float out, float fps, std::string label){

    writer.StartObject();
    writer.String("label");
    writer.String(label.c_str());
    writer.String("tcin");
    writer.String(frames2tc(in,fps).c_str());
    writer.String("tcout");
    writer.String(frames2tc(out,fps).c_str());
    writer.String("tclevel");
    writer.Double(1.0);
    writer.EndObject();
}

void overlay(PrettyWriter<StringBuffer>& writer, float in, float out, float fps, float x, float y, float rx, float ry, std::string label){
    std::vector<float> tcs;
    tcs.push_back(in);
    tcs.push_back(out);

    for( std::vector<float>::iterator tc = tcs.begin(); tc != tcs.end(); tc++){
        writer.StartObject();
        writer.String("label");
        writer.String(label.c_str());
        writer.String("shape");
        writer.StartObject();
        writer.String("t");
        writer.String("rectangle");
        writer.String("c");
        writer.StartObject();
        writer.String("x");
        writer.Double(x);
        writer.String("y");
        writer.Double(y);
        writer.EndObject();
        writer.String("rx");
        writer.Double(rx);
        writer.String("ry");
        writer.Double(ry);
        writer.String("o");
        writer.Double(0.0);
        writer.EndObject();
        writer.String("tc");
        writer.String(frames2tc(*tc,fps).c_str());
        writer.String("tclevel");
        writer.Double(1.0);
        writer.EndObject();
    }

}

void eventfooter(PrettyWriter<StringBuffer>& writer, std::string id, std::string source, float out, float fps){
    writer.EndArray();
    writer.EndObject();

    writer.String("type");
    writer.String("events");
    writer.String("tcin");
    writer.String("00:00:00.0000");
    writer.String("tcout");
    writer.String(frames2tc(out,fps).c_str());
    writer.String("tclevel");
    writer.Double(0);

    writer.EndObject();
    writer.EndArray();

    writer.String("id");
    writer.String((id+"-events").c_str());
    writer.String("type");
    writer.String("events");

    writer.String("algorithm");
    writer.String("InspectorWidget Events");
    writer.String("processor");
    writer.String("InspectorWidget Processor");
    writer.String("source");
    writer.String(source.c_str());
    writer.String("processed");
    writer.Uint64(11421141589286);
    writer.String("version");
    writer.Double(1);

    writer.EndObject();

}

void segmentfooter(PrettyWriter<StringBuffer>& writer, std::string id, std::string source, float out, float fps){
    writer.EndArray();
    writer.EndObject();

    writer.String("type");
    writer.String("segments");
    writer.String("tcin");
    writer.String("00:00:00.0000");
    writer.String("tcout");
    writer.String(frames2tc(out,fps).c_str());
    writer.String("tclevel");
    writer.Double(0);

    writer.EndObject();
    writer.EndArray();

    writer.String("id");
    writer.String((id+"-segments").c_str());
    writer.String("type");
    writer.String("segments");

    writer.String("algorithm");
    writer.String("InspectorWidget Segments");
    writer.String("processor");
    writer.String("InspectorWidget Processor");
    writer.String("source");
    writer.String(source.c_str());
    writer.String("processed");
    writer.Uint64(11421141589286);
    writer.String("version");
    writer.Double(1);

    writer.EndObject();

}

void overlayfooter(PrettyWriter<StringBuffer>& writer, std::string id, std::string source, float out, float fps){
    writer.EndArray();
    writer.EndObject();

    writer.String("label");
    writer.String((id+"-overlays").c_str());
    writer.String("tcin");
    writer.String("00:00:00.0000");
    writer.String("tcout");
    writer.String(frames2tc(out,fps).c_str());
    writer.String("tclevel");
    writer.Double(0);

    writer.EndObject();
    writer.EndArray();

    writer.String("id");
    writer.String((id+"-overlays").c_str());
    writer.String("type");
    writer.String("visual_tracking");

    writer.String("algorithm");
    writer.String("InspectorWidget Overlays");
    writer.String("processor");
    writer.String("InspectorWidget Processor");
    writer.String("source");
    writer.String(source.c_str());
    writer.Uint64(11421141589286);
    writer.String("processed");
    writer.String("version");
    writer.Double(1);

    writer.EndObject();

}

std::string getStemName(std::string file_path){
#ifdef WIN32
    std::string slash("\\");
#else
    std::string slash("/");
#endif
    int lastslash = file_path.find_last_of(slash);
    std::string stemext = file_path.substr(lastslash+1, file_path.size());
    int lastpoint = stemext.find_last_of(".");
    std::string stem = stemext.substr(0, lastpoint);
    return stem;
}

std::string getExtension(std::string file_path){
    int lastpoint = file_path.find_last_of(".");
    if(lastpoint == std::string::npos) return "";
    std::string ext = file_path.substr(lastpoint+1, file_path.size());
    return ext;
}

InspectorWidgetProcessor::InspectorWidgetProcessor(){
    status_phase = "constructor";
    this->clear();

    parser_operators = 0;
    //Methods:
    //0: SQDIFF
    //1: SQDIFF NORMED
    //2: TM CCORR
    //3: TM CCORR NORMED
    //4: TM COEFF
    //5: TM COEFF NORMED
    match_method = 5; //for fast 5 (me) 3(web)

    // TM COEFF NORMED no thres color/gray
    // TM CCORR NORMED thres gray

    max_Trackbar = 5;

    with_gui = false;
    image_window = "Source Image";
    result_window = "Result window";
    text_window = "Detected text";

    _threshold = 0.99;

    supported_extraction_tests.push_back("if");
    supported_extraction_tests.push_back("between");
    supported_extraction_tests.push_back("below");
    supported_extraction_tests.push_back("rightof");
    supported_extraction_tests.push_back("inrect");

    supported_extraction_actions.push_back("matchTemplate");
    supported_extraction_actions.push_back("detectText");
    supported_extraction_actions.push_back("detectNumber");
    supported_extraction_actions.push_back("detectTime");
    supported_extraction_actions.push_back("template");

    supported_conversion_tests.push_back("during");
    supported_conversion_tests.push_back(""); // make test statements optional

    supported_conversion_actions.push_back("matchFirstValueOf");
    supported_conversion_actions.push_back("triggerBySegmentsOf");
    supported_conversion_actions.push_back("eval");
    supported_conversion_actions.push_back("nestByLastVariable");
    //supported_conversion_actions.push_back("append");
    //supported_conversion_actions.push_back("ifThenElse");
    //supported_conversion_actions.push_back("ifThen");

    //supported_accessibility_tests.push_back("during");
    supported_accessibility_tests.push_back(""); // make test statements optional

    supported_accessibility_actions.push_back("getFocusApplication");
    supported_accessibility_actions.push_back("getFocusWindow");
    supported_accessibility_actions.push_back("getPointedWidget");
    supported_accessibility_actions.push_back("getWorkspaceSnapshot");

    //supported_input_hook_tests.push_back("during");
    supported_input_hook_tests.push_back(""); // make test statements optional

    supported_input_hook_actions.push_back("getWords");
    supported_input_hook_actions.push_back("getPointerClicks");
    supported_input_hook_actions.push_back("getKeysTyped");
    supported_input_hook_actions.push_back("getModifierKeysPressed");
    //supported_input_hook_actions.push_back("matchKey");
    //supported_input_hook_actions.push_back("getInputInactivity"); // with user-definable temporal threshold
}

void InspectorWidgetProcessor::clear(){
    active = false;
    status_progress = -1;
    status_success = "";
    status_error = "";
    active = false;
    datapath = "";
    videostem = "";
    frame = 0;
    csv_frame = 0;
    video_w = 0;
    video_h = 0;
    video_frames = 0;
    current_frame = 0;
    start_h = 0;
    start_m = 0;
    start_clock = 0;
    end_clock = 0;
    first_minute_frames = -1;

    ts_time.clear();
    ts_clock.clear();
    
    templates.clear();
    gray_templates.clear();
    template_x.clear();
    template_y.clear();
    template_val.clear();
    // template_vals.clear(); // make it persistent
    template_w.clear();
    template_h.clear();
    logged_template_w.clear();
    logged_template_h.clear();
    text_x.clear();
    text_y.clear();
    text_txt.clear();
    csvfile.clear();
    log_val.clear();
    log_txt.clear();
    log_x.clear();
    log_y.clear();
    
    start_t = InspectorWidgetTime();
    end_t = InspectorWidgetTime();
    start_d = InspectorWidgetDate();
    
    template_matching_logged_dep_list.clear();
    template_matching_logged_dep_map.clear();

    template_matching_new_dep_list.clear();
    template_matching_new_dep_map.clear();

    template_matching_dep_map.clear();

    text_detect_logged_dep_list.clear();
    text_detect_logged_dep_map.clear();

    text_detect_new_dep_list.clear();
    text_detect_new_dep_map.clear();

    text_detect_dep_map.clear();

    template_list.clear();

    text_detect_list.clear();

    template_matching_type.clear();
    template_matching_test.clear();

    text_detect_type.clear();
    text_detect_test.clear();

    inrect_map.clear();

    filters.clear();
    filtering_test.clear();
    filtering_deps.clear();
    filtering_action.clear();
    filtering_variables.clear();

    inputhooks.clear();
    inputhook_test.clear();
    inputhook_deps.clear();
    inputhook_action.clear();
    inputhook_variables.clear();

    ax_annotations.clear();
    ax_test.clear();
    ax_deps.clear();
    ax_action.clear();
    ax_variables.clear();

    ax_hover_time = -1;
    ax_hover_x = -1; /// check with dual monitors
    ax_hover_y = -1; /// check with dual monitors
    ax_hover_rect.clear();
    ax_hover_tree_children = "";
    ax_hover_tree_parents = "";
    /*ax_hover_root = pugi::xml_node();
    ax_hover_root_parsed = false;
    ax_hover_closest_node = pugi::xml_node();
    ax_hover_closest_window_node = pugi::xml_node();
    ax_hover_closest_parsed = false;*/
}

InspectorWidgetProcessor::~InspectorWidgetProcessor(){
    file.close();
    for(std::map<std::string,std::ofstream>::iterator _csvfile = csvfile.begin(); _csvfile!=csvfile.end();_csvfile++){
        _csvfile->second.close();
    }
}

//std::map<std::string, std::vector<float> > InspectorWidgetProcessor::parseCSV(std::string file){
std::map< std::string, std::map<std::string, std::vector<float> > > InspectorWidgetProcessor::parseCSV(std::string file){
    std::map< std::string, std::map<std::string, std::vector<float> > > data;
    std::map<std::string, std::vector<float> > val;
    std::map<std::string, std::vector<float> > x;
    std::map<std::string, std::vector<float> > y;

    try{
        PCP::CsvConfig csv_config (file.c_str());

        // parse header line
        std::vector<std::string> headers = csv_config.get_headers();

        int annotations = (headers.size()-1)/3.0;
        std::cout << annotations << std::endl;

        std::vector<std::string> label(annotations);
        std::vector<int> in(annotations,0);
        //std::vector<float> val(annotations,0.0);

        for(size_t i = 0; i < annotations; ++i){
            std::string _label = headers[3*i+3];
            _label.erase (std::remove(_label.begin(), _label.end(), '\"'), _label.end());

            std::size_t pos = _label.find("_val");
            if(pos!=std::string::npos)
                _label = _label.substr (0,pos);

            label[i] = _label;

            std::cout << label[i] << std::endl;
        }
        std::cout << std::endl;

        // print headers
        std::cout << "Headers:" << std::endl;
        for (size_t i = 0; i < headers.size(); ++i)
            std::cout << headers[i] << "\t" /*<< (i-1)%3 << "\n"*/;
        std::cout << std::endl;

        // instantiate parser
        PCP::PartialCsvParser parser(csv_config);  // parses whole body of CSV without range options.

        int r = 0;
        int f = 0;

        // parse & print body lines
        std::vector<std::string> row;
        while (!(row = parser.get_row()).empty()) {

            /*            std::cout << "Got a row: ";
            for (size_t i = 0; i < row.size(); ++i){
                std::cout << row[i] << "\t";
            }
            //std::cout << std::endl;
*/
            if( row.size() == headers.size()){
                int _in = atoi(row[0].c_str());
                f = _in;
                //std::cout << " frame=" << _in << "\t";
                for(size_t i = 0; i < annotations; ++i){

                    float _val = atof(row[3*i+3].c_str());
                    _val = (_val>_threshold)?1:0;
                    //std::cout << " _val='" << _val << "'' ";

                    float _x = atof(row[3*i+1].c_str());
                    float _y = atof(row[3*i+2].c_str());

                    val[label[i]].push_back(_val);
                    x[label[i]].push_back(_x);
                    y[label[i]].push_back(_y);
                    /*if ( val[i] != _val){

                        std::cout << "row " << r << " label=" << label[i] << " samples="  << _in - in[i] << " val='" << val[i] << "'" << std::endl;;

                        if(val[i] > _threshold){
                            //segment(*(writer[i]), in[i], _in);
                        }

                        in[i] = _in;
                        val[i] = _val;
                    }*/
                    //std::cout << " " << row[i] << "\t";
                }
            }
            r++;
        }

        /*for(size_t i = 0; i < annotations; ++i){
            if(val[i] > _threshold){
                //segment(*(writer[i]), in[i], f);
            }
            //std::cout << " " << row[i] << "\t";
        }*/


    }
    catch(...){

    }
    data["x"] = x;
    data["y"] = y;
    data["val"] = val;
    return data;
}

void MatchingMethod( cv::Mat& srca,  // The reference image
                     cv::Mat& srcb,  // The template image
                     cv::Mat& dst,   // Template matching result
                     int match_method)
{
    /// Create the result matrix
    cv::Mat result;
    int result_cols =  srca.cols - srcb.cols + 1;
    int result_rows = srca.rows - srcb.rows + 1;

    result.create( result_rows, result_cols, CV_32FC1 );

    /// Do the Matching and Normalize
    matchTemplate( srca, srcb, result, match_method );

    //CF normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

    result.copyTo(dst);
}

void fastMatchTemplate(cv::Mat& srca,  // The reference image
                       cv::Mat& srcb,  // The template image
                       cv::Mat& dst,   // Template matching result
                       int maxlevel,   // Number of levels
                       int match_method)
{
    std::vector<cv::Mat> refs, tpls, results;

    // Build Gaussian pyramid
    cv::buildPyramid(srca, refs, maxlevel);
    cv::buildPyramid(srcb, tpls, maxlevel);

    cv::Mat ref, tpl, res;

    // Process each level
    for (int level = maxlevel; level >= 0; level--)
    {
        ref = refs[level];
        tpl = tpls[level];
        res = cv::Mat::zeros(ref.size() + cv::Size(1,1) - tpl.size(), CV_32FC1);


        if (level == maxlevel)
        {
            // On the smallest level, just perform regular template matching
            cv::matchTemplate(ref, tpl, res, match_method  /*TM_CCORR_NORMED*/);
        }
        else
        {
            // On the next layers, template matching is performed on pre-defined
            // ROI areas.  We define the ROI using the template matching result
            // from the previous layer.

            cv::Mat mask;
            cv::pyrUp(results.back(), mask);

            cv::Mat mask8u;
            mask.convertTo(mask8u, CV_8U);

            // Find matches from previous layer
            std::vector<std::vector<cv::Point> > contours;
            cv::findContours(mask8u, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

            // Use the contours to define region of interest and
            // perform template matching on the areas
            for (int i = 0; i < contours.size(); i++)
            {
                cv::Rect r = cv::boundingRect(contours[i]);
                cv::Rect rr = r + (tpl.size() - cv::Size(1,1));
                if(rr.x+rr.width<=ref.cols && rr.y+rr.height<=ref.rows){
                    cv::matchTemplate(
                                ref(r + (tpl.size() - cv::Size(1,1))),
                                tpl,
                                res(r),
                                match_method  /*TM_CCORR_NORMED*/
                                );
                }
            }
        }

        // Only keep good matches
        //CF cv::threshold(res, res, 0.9f, 1.0f, THRESH_TOZERO); // not working...
        results.push_back(res);
    }

    res.copyTo(dst);
}

void InspectorWidgetProcessor::matchTemplates(){

    cv::cvtColor(img, ref_gray, COLOR_BGR2GRAY);

    /*std::map<std::string,bool> _match_template;
    for(std::vector<std::string>::iterator _name = template_list.begin(); _name != template_list.end(); _name++ ){
        _match_template[*_name] = (inrect_map.find(*_name)!=inrect_map.end());
    }

    for(std::map<std::string,std::vector<std::string> >::iterator _m = template_matching_logged_dep_map.begin(); _m != template_matching_logged_dep_map.end(); _m++){
        bool _match = true;
        for(std::vector<std::string>::iterator _v = _m->second.begin(); _v != _m->second.end(); _v++){
            if(log_val[*_v][csv_frame]<_threshold){
                _match &= false;
            }
            if(template_val[*_v]<_threshold){
                _match &=false;
            }
        }
        _match_template[_m->first] = _match;
    }

    for(std::list<std::string>::iterator _t = template_matching_new_dep_list.begin(); _t != template_matching_new_dep_list.end();_t++){
        _match_template[*_t] = true;
    }

    for(std::list<std::string>::iterator _t = text_detect_new_dep_list.begin(); _t != text_detect_new_dep_list.end();_t++){
        _match_template[*_t] = true;
    }*/

    //for(std::list<std::string>::iterator _t = _templates.begin(); _t != _templates.end();_t++){
    for(std::vector<std::string>::iterator _n = template_list.begin(); _n != template_list.end(); _n++ ){

        std::string _name = *_n;

        bool _match_template = (inrect_map.find(_name)!=inrect_map.end());

        std::vector<std::string> logged_deps = template_matching_logged_dep_map[_name];
        bool logged_match = true;
        for(std::vector<std::string>::iterator _d = logged_deps.begin(); _d != logged_deps.end(); _d++){
            if(log_val[*_d][csv_frame]<_threshold){
                logged_match &= false;
            }
        }
        if(logged_deps.size()>0) _match_template = logged_match;

        std::vector<std::string> new_deps = template_matching_new_dep_map[_name];
        bool new_match = true;
        for(std::vector<std::string>::iterator _d = new_deps.begin(); _d != new_deps.end(); _d++){
            if(template_val[*_d]<_threshold){
                new_match &= false;
            }
        }
        if(new_deps.size()>0) _match_template = new_match;

        if(template_matching_test[_name].empty()) _match_template = true;

        //if( std::find(template_matching_new_dep_list.begin(),template_matching_new_dep_list.end(),_name) != template_matching_new_dep_list.end() ) _match_template = true;
        //if( std::find(text_detect_new_dep_list.begin(),text_detect_new_dep_list.end(),_name) != text_detect_new_dep_list.end() ) _match_template = true;

        //if(_match_template[_name] == true){
        if(_match_template){

            cv::Rect rect;
            cv::Mat _frame;
            cv::Mat _template;
            bool gray = true;

            if( template_matching_test[_name] == "inrect"){
                std::vector<int> _rect = this->inrect_map[_name];
                rect.x = _rect[0];
                rect.y = _rect[1];
                rect.width = _rect[2];
                rect.height = _rect[3];
            }
            else if(template_matching_test[_name] == "below" || template_matching_test[_name] == "rightof"){

                std::vector<std::string> deps = template_matching_dep_map[_name];
                if(deps.size()!=1){
                    std::cerr << "Template to match '" << _name << "' should have only one dependency for test 'below', not extracting..." << std::endl;
                    return; //exit(0);
                    //break;
                }
                std::string _depname = deps[0];

                std::map<std::string, std::vector<float> >::iterator is_logged = log_val.find(_depname);
                std::vector<std::string>::iterator is_template = std::find(template_list.begin(),template_list.end(),_depname);

                if(template_matching_test[_name] == "below"){
                    if(is_logged!=log_val.end()){
                        rect.x = log_x[_depname][csv_frame];
                        rect.y = log_y[_depname][csv_frame]+logged_template_h[_depname];
                        rect.width = logged_template_w[_depname];
                        rect.height = img.rows - log_y[_depname][csv_frame] - logged_template_h[_depname];
                    }
                    else if(is_template!=template_list.end()){
                        rect.x = template_x[_depname];
                        rect.y = template_y[_depname]+template_h[_depname];
                        rect.width = template_w[_depname];
                        rect.height = img.rows - template_y[_depname] - template_h[_depname];
                    }
                    else{
                        std::cerr << "Test variable " << _depname << " is neither part of logged templates nor templates to be extracted, aborting" << std::endl;
                        return; //exit(0);
                    }
                }
                else if(template_matching_test[_name] == "rightof"){
                    if(is_logged!=log_val.end()){
                        rect.x = log_x[_depname][csv_frame]+logged_template_w[_depname];
                        rect.y = log_y[_depname][csv_frame];
                        rect.width = img.cols - log_x[_depname][csv_frame] - logged_template_w[_depname];
                        rect.height = logged_template_h[_depname];
                    }
                    else if(is_template!=template_list.end()){
                        rect.x = template_x[_depname]+template_w[_depname];
                        rect.y = template_y[_depname];
                        rect.width = img.cols - template_x[_depname] - template_w[_depname];
                        rect.height = template_h[_depname];
                    }
                    else{
                        std::cerr << "Test variable " << _depname << " is neither part of logged templates nor templates to be extracted, aborting" << std::endl;
                        return; //exit(0);
                    }
                }
            }

            if( template_matching_test[_name] == "inrect" || template_matching_test[_name] == "below" || template_matching_test[_name] == "rightof"){
                if(gray){
                    _frame = ref_gray(rect);
                }
                else{
                    _frame = img(rect);
                }
                std::string imagefile = this->datapath + this->videostem + "-" + _name + ".png";
                cv::imwrite(imagefile.c_str(),_frame);
            }
            else{
                if(gray){
                    _frame = ref_gray;
                }
                else{
                    _frame = img;
                }
            }

            if(gray){
                _template = gray_templates[_name];
            }
            else{
                _template = templates[_name];
            }

            int fastmatchlevels = 2;
            if( _frame.cols - _template.cols <= 1 || _frame.rows - _template.rows <= 1 ){ // pyramid removes 1 px per dim
                fastmatchlevels = 0;
            }

            //MatchingMethod( _frame, _template, dst, match_method);
            fastMatchTemplate(_frame, _template, dst, fastmatchlevels, match_method);

            /// Localizing the best match with minMaxLoc
            double minVal; double maxVal; Point minLoc; Point maxLoc;
            double matchVal; Point matchLoc;

            minMaxLoc( dst, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

            /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
            if( match_method  == TM_SQDIFF || match_method == TM_SQDIFF_NORMED )
            { matchLoc = minLoc; matchVal = minVal; }
            else
            { matchLoc = maxLoc; matchVal = maxVal; }

            std::cout << "Match location for '"<< _name << "': x=" <<  matchLoc.x << " y=" <<  matchLoc.y  << " with minVal=" << minVal << " maxVal=" << maxVal <<" matchVal=" << matchVal <<std::endl;

            if(template_matching_test[_name] == "below" || template_matching_test[_name] == "rightof"){

                template_x[_name] = rect.x + matchLoc.x;
                template_y[_name] = rect.y + matchLoc.y;
                template_val[_name] = matchVal;

            }
            else{

                template_x[_name] = matchLoc.x;
                template_y[_name] = matchLoc.y;
                template_val[_name] = matchVal;

            }
            
            if(template_vals[_name].size() == 0){
                std::cout << "Init storage of values from template " << _name << std::endl;
                template_vals[_name].resize(this->video_frames,0.0);
            }
            
            template_vals[_name][frame] = matchVal;

            //CF

            if(with_gui && matchVal>_threshold){
                cv::rectangle(
                            img, matchLoc,
                            cv::Point(matchLoc.x + templates[_name].cols, matchLoc.y + templates[_name].rows),
                            cv::Scalar(0,255,0), 2
                            );
                cv::floodFill(
                            img, matchLoc,
                            cv::Scalar(0), 0,
                            cv::Scalar(.1),
                            cv::Scalar(1.)
                            );
            }
        }
        else{
            template_x[_name] = 0;
            template_y[_name] = 0;
            template_val[_name] = 0;

        }
        file << "," << template_x[_name];
        file << "," << template_y[_name];
        file << "," << template_val[_name];

        csvfile[_name] << frame;
        csvfile[_name] << "," << template_x[_name];
        csvfile[_name] << "," << template_y[_name];
        csvfile[_name] << "," << template_val[_name];
        csvfile[_name] << std::endl;

    }
}


bool InspectorWidgetProcessor::detectText(){

    /*std::map<std::string,bool> _detect_text;
    for(std::set<std::string>::iterator _name = text_detect_list.begin(); _name != text_detect_list.end(); _name++ ){
        _detect_text[*_name] = (inrect_map.find(*_name)!=inrect_map.end());
    }
    for(std::map<std::string,std::vector<std::string> >::iterator _m = text_detect_logged_dep_map.begin(); _m != text_detect_logged_dep_map.end(); _m++){
        bool _match = true;
        for(std::vector<std::string>::iterator _v = _m->second.begin(); _v != _m->second.end(); _v++){
            //std::cout <<  _m->first << "<-" << *_v << " "<< log_val[*_v][csv_frame] << std::endl;
            if(log_val[*_v][csv_frame]<_threshold){
                _match &= false;
            }
        }
        _detect_text[_m->first] = _match;
    }*/
    /* CF to redo!
    for(std::map<std::string,std::vector<std::string> >::iterator _m = text_detect_new_dep_map.begin(); _m != text_detect_new_dep_map.end(); _m++){
        bool _match = true;
        for(std::vector<std::string>::iterator _v = _m->second.begin(); _v != _m->second.end(); _v++){
            //std::cout <<  _m->first << "<-" <<   *_v << " "<< template_val[*_v] << std::endl;
            if(template_val[*_v]<_threshold){
                _match &= false;
            }
        }
        _detect_text[_m->first] = _match;
    }
*/

    for(std::set<std::string>::iterator _n= text_detect_list.begin(); _n != text_detect_list.end(); _n++ ){

        std::string _name = *_n;

        bool _detect_text = (inrect_map.find(_name)!=inrect_map.end());;

        std::vector<std::string> logged_deps = text_detect_logged_dep_map[_name];
        bool logged_match = true;
        for(std::vector<std::string>::iterator _d = logged_deps.begin(); _d != logged_deps.end(); _d++){
            if(log_val[*_d][csv_frame]<_threshold){
                logged_match &= false;
            }
        }
        if(logged_deps.size()>0) _detect_text = logged_match;

        std::vector<std::string> new_deps = text_detect_new_dep_map[_name];
        bool new_match = true;
        for(std::vector<std::string>::iterator _d = new_deps.begin(); _d != new_deps.end(); _d++){
            if(template_val[*_d]<_threshold){
                new_match &= false;
            }
        }
        if(new_deps.size()>0) _detect_text = new_match;

        std::string text(" ");
        float x(0.0),y(0.0);

        //if(_detect_text[_name] == true){
        if(_detect_text == true){

            cv::Rect rect;

            if( text_detect_test[_name] == "inrect"){
                std::vector<int> _rect = this->inrect_map[_name];
                rect.x = _rect[0];
                rect.y = _rect[1];
                rect.width = _rect[2];
                rect.height = _rect[3];
            }
            else if (text_detect_test[_name] == "between"){

                std::vector<float> _xs,_ys;
                std::vector<int> _ws,_hs;

                for(std::vector<std::string>::iterator _l = text_detect_logged_dep_map[_name].begin(); _l != text_detect_logged_dep_map[_name].end(); _l++ ){
                    _xs.push_back( log_x[*_l][csv_frame] );
                    _ys.push_back( log_y[*_l][csv_frame] );
                    _ws.push_back( logged_template_w[*_l] );
                    _hs.push_back( logged_template_h[*_l] );
                }
                for(std::vector<std::string>::iterator _l = text_detect_new_dep_map[_name].begin(); _l != text_detect_new_dep_map[_name].end(); _l++ ){
                    _xs.push_back( template_x[*_l] );
                    _ys.push_back( template_y[*_l] );
                    _ws.push_back( template_w[*_l] );
                    _hs.push_back( template_h[*_l] );
                }
                if(_xs.size()!=2 || _ys.size()!=2 ){
                    std::cerr << "Can only detect text between 2 matched templates" << std::endl;
                    return false;
                }

                /*{
                int i = 0;
                cv::Rect _rect;
                _rect.x = _xs[i];
                _rect.y = _ys[i];
                _rect.width = _ws[i];
                _rect.height = _hs[i];
                cv::Mat _image  = cv::Mat(img,_rect).clone();
                imshow("0", _image );
                waitKey(1);
            }

            {
                int i = 1;
                cv::Rect _rect;
                _rect.x = _xs[i];
                _rect.y = _ys[i];
                _rect.width = _ws[i];
                _rect.height = _hs[i];
                cv::Mat _image  = cv::Mat(img,_rect).clone();
                imshow("1", _image );
                waitKey(1);
            }*/


                // Determining an horizontal region between both templates

                float _l,_t,_b,_r;

                if(_xs[0] > _xs[1] ){
                    _l = _xs[1] + _ws[1];
                    _r = _xs[0];
                }
                else{
                    _l = _xs[0] + _ws[0];
                    _r = _xs[1];
                }

                // Using the union of heights
                //_t = min( _ys[0], _ys[1] );
                //_b = max( _ys[0] + _hs[0], _ys[1] + _hs[1] );

                // Using the intersections of heights
                _t = (_ys[0] + _hs[0] < _ys[1] + _hs[1]) ? _ys[0] : _ys[1];
                _b = (_ys[0] + _hs[0] < _ys[1] + _hs[1]) ? _ys[0] + _hs[0] : _ys[1] + _hs[1];


                rect.x = _l;
                rect.y = _t;
                rect.width = _r-_l;
                rect.height = _b-_t;

            }
            else{
                std::cerr << "Test " << text_detect_test[_name] << " not implemented for text detection, aborting" << std::endl;
                return 0; //exit(0);
            }

            if(with_gui){
                cv::Mat imz  = img.clone();

                //            cv::rectangle(
                //                        imz, cv::Point(_xs[0] , _ys[0]),
                //                    cv::Point(_xs[0]+_ws[0] , _ys[0]+_hs[0]),
                //                    cv::Scalar(0,255,0), 2
                //                    );
                //            cv::floodFill(
                //                        imz, cv::Point(_xs[0] , _ys[0]),
                //                    cv::Scalar(0), 0,
                //                    cv::Scalar(.1),
                //                    cv::Scalar(1.)
                //                    );

                //            cv::rectangle(
                //                        imz, cv::Point(_xs[1] , _ys[1]),
                //                    cv::Point(_xs[1]+_ws[1] , _ys[1]+_hs[1]),
                //                    cv::Scalar(255,0,0), 2
                //                    );
                //            cv::floodFill(
                //                        imz, cv::Point(_xs[1] , _ys[1]),
                //                    cv::Scalar(0), 0,
                //                    cv::Scalar(.1),
                //                    cv::Scalar(1.)
                //                    );

                cv::rectangle(
                            imz, rect,
                            cv::Scalar(0,255,0), 2
                            );
                cv::floodFill(
                            imz, cv::Point(rect.x , rect.y),
                            cv::Scalar(0), 0,
                            cv::Scalar(.1),
                            cv::Scalar(1.)
                            );


                imshow("orig", imz );
                waitKey(0);
            }

            cv::Mat image = img(rect);
            cvtColor( image, image, COLOR_RGB2GRAY);


            if(!image.empty()){
                //namedWindow( text_window, WINDOW_KEEPRATIO | WINDOW_NORMAL );
                if(with_gui){
                    imshow(text_window, image );
                    waitKey(0);
                }


                if( text_detect_type[_name] == "detectTime" || text_detect_type[_name] == "detectText"){
                    //CF threshold test
                    /* 0: Binary
                     1: Binary Inverted
                     2: Threshold Truncated
                     3: Threshold to Zero
                     4: Threshold to Zero Inverted
                   */

                    int threshold_type = 0;
                    double max_BINARY_value = 255;
                    double threshold_value = 0.5*255;

                    threshold( image, image, threshold_value, max_BINARY_value,threshold_type );
                }

                std::string imagefile = this->datapath + this->videostem + "-detecttext.png";
                cv::imwrite(imagefile.c_str(),image);

                /*std::stringstream patchpath;
                patchpath << datapath << videostem << "-patch-" << csv_frame << ".png";
                cv::imwrite(patchpath.str(),image);*/

                std::string lang("eng");
                //std::cout << "Using language " << lang << std::endl;

                /* Use Tesseract to try to decipher our image */
                tesseract::TessBaseAPI tesseract_api;

                tesseract_api.Init(NULL, lang.c_str() );

                if( text_detect_type[_name] == "detectNumber"){
                    //CF digit detection test
                    tesseract_api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
                    bool digitsonly = tesseract_api.SetVariable("tessedit_char_whitelist", "0123456789");
                    //std::cout << "digitsonly "<< digitsonly << std::endl;
                }
                else if( text_detect_type[_name] == "detectTime"){
                    //CF digit detection test
                    tesseract_api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
                    bool digitsonly = tesseract_api.SetVariable("tessedit_char_whitelist", "0123456789:-.");
                    //std::cout << "digitsonly "<< digitsonly << std::endl;
                }
                else if( text_detect_type[_name] == "detectText"){
                    tesseract_api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
                    bool alphasonly = tesseract_api.SetVariable("tessedit_char_whitelist", "abcdefghijklmnopqrstuvxyz");
                    //std::cout << "alphasonly "<< alphasonly << std::endl;
                }

                //tesseract_api.SetPageSegMode(tesseract::PSM_AUTO_ONLY);

                tesseract_api.SetImage((uchar*) image.data, image.cols, image.rows, 1, image.cols);


                text = string(tesseract_api.GetUTF8Text());

                //std::cout << "Text: " << text << std::endl;

                /* Split the string by whitespace */
                vector<string> splitted;
                istringstream iss( text );
                copy( istream_iterator<string>(iss), istream_iterator<string>(), back_inserter( splitted ) );

                int n_words = splitted.size();
                std::cout << n_words << " word(s)" << std::endl;

                std::cout << "Detected text: '" << text << "'" << std::endl;

                if(text.empty())
                    text = " ";

                text.erase (std::remove(text.begin(), text.end(), '\n'), text.end());
                text.erase (std::remove(text.begin(), text.end(), ','), text.end());

                if( text_detect_type[_name] == "detectNumber"){
                    if(n_words!=1){
                        text = " ";
                    }
                    else{
                        text = splitted[0];
                    }
                }
                //                else if( text_detect_type[_name] == "detectTime"){

                //                }

                std::cout << "Processed text: '" << text << "'" << std::endl;
                x = rect.x;
                y = rect.y;


            }
        }

        text_txt[_name] = text;
        text_x[_name] = x;
        text_y[_name] = y;

        file << "," << text_x[_name];
        file << "," << text_y[_name];
        file << "," << text_txt[_name];

        csvfile[_name] << frame;
        csvfile[_name] << "," << text_x[_name];
        csvfile[_name] << "," << text_y[_name];
        csvfile[_name] << "," << text_txt[_name];
        csvfile[_name] << std::endl;

    }
    return true;
}


std::string InspectorWidgetProcessor::getStatusError(){
    return status_error;
}

std::string InspectorWidgetProcessor::getStatusSuccess(){
    return status_success;
}

std::string InspectorWidgetProcessor::getStatusPhase(){
    return status_phase;
}

float InspectorWidgetProcessor::getStatusProgress(){
    return status_progress;
}

bool InspectorWidgetProcessor::setStatusAndReturn(std::string phase, std::string error_message, std::string success_message ){
    this->status_phase = phase;
    this->status_success = "";
    this->status_error = "";
    if(error_message.empty() && !success_message.empty()){
        status_success = success_message;
        std::cout << success_message << std::endl;
        return true;
    }
    else if(!error_message.empty() && success_message.empty()){
        status_error = error_message;
        status_progress = -1;
        active = false;
        std::cerr << error_message << std::endl;
        return false;
    }
    else{
        status_error = "error";
        status_progress = -1;
        active = false;
        //throw
        std::cerr << "Unhandled status, aborting" << std::endl;
        return false;
    }
}

bool InspectorWidgetProcessor::init( int argc, char** argv ){
    if(argc<3){
        return setStatusAndReturn(
                    /*phase*/ "init",
                    /*error*/ std::string("Usage: ")+std::string(argv[0])+std::string(" <data_path> <source_video> <template_img_1> [<template_img_2> ...] [<csv>] [template_constraint]"),
                /*success*/ ""
                );
    }

    std::vector<std::string> args;
    for(int a=1; a<argc;a++){
        args.push_back(argv[a]);
    }
    return this->init(args);
} 

bool InspectorWidgetProcessor::init( std::vector<std::string> argv ){

    std::string _videostem;
    _videostem = getStemName(argv[1]);
    std::size_t underpos = _videostem.find_first_of("_");
    if(underpos!=std::string::npos)
        _videostem = _videostem.substr(0,underpos);
    if(videostem != _videostem){
        this->clear();
    }
    videostem = _videostem;

    status_phase = "init";
    status_progress = 0;
    active = true;

    /// Init lexical parser
    pegtl::analyze< InspectorWidgetProcessorCommandParser::grammar >();
    parser_operators = new InspectorWidgetProcessorCommandParser::operators( std::bind( &InspectorWidgetProcessor::s2b, this, std::placeholders::_1), std::bind( &InspectorWidgetProcessor::b2s, this, std::placeholders::_1) );

    /// Setup default file path/stem:
    datapath = argv[0];
#ifdef WIN32
    std::string slash("\\");
#else
    std::string slash("/");
#endif
    std::size_t slashpos = datapath.find_last_of(slash);
    if(slashpos!=datapath.size()-1)
        datapath += slash;

    std::string videopath = datapath + videostem + ".mp4";
    std::cout << "datapath " << datapath << std::endl;
    std::cout << "videostem " << videostem << std::endl;
    std::cout << "videopath " << videopath << std::endl;

    /// Check if date and time are encoded in the filename:
    std::string _timestamps = videostem;
    /// split the video filename in a vector by delimiter '-'
    vector<string> _splitstamps = splitconstraint (_timestamps, '-');
    if(_splitstamps.size()>=6 & _splitstamps[_splitstamps.size()-6].size() == 4 ){
        start_d.y = atoi(_splitstamps[_splitstamps.size()-6].substr(0,1).c_str())*1000
                + atoi(_splitstamps[_splitstamps.size()-6].substr(1,1).c_str())*100
                + atoi(_splitstamps[_splitstamps.size()-6].substr(2,1).c_str())*10
                + atoi(_splitstamps[_splitstamps.size()-6].substr(3,1).c_str());
        start_d.m = atoi(_splitstamps[_splitstamps.size()-5].substr(0,1).c_str())*10
                + atoi(_splitstamps[_splitstamps.size()-5].substr(1,1).c_str());
        start_d.d = atoi(_splitstamps[_splitstamps.size()-4].substr(0,1).c_str())*10
                + atoi(_splitstamps[_splitstamps.size()-4].substr(1,1).c_str());
        start_t.h = atoi(_splitstamps[_splitstamps.size()-3].substr(0,1).c_str())*10
                + atoi(_splitstamps[_splitstamps.size()-3].substr(1,1).c_str());
        start_t.m = atoi(_splitstamps[_splitstamps.size()-2].substr(0,1).c_str())*10
                + atoi(_splitstamps[_splitstamps.size()-2].substr(1,1).c_str());
        start_t.s = atoi(_splitstamps[_splitstamps.size()-1].substr(0,1).c_str())*10
                + atoi(_splitstamps[_splitstamps.size()-1].substr(1,1).c_str());
    }
    std::cout << "Timestamp encoded in file name: '" << start_d.y << "/" << start_d.m << "/"  << start_d.d << "' '";
    std::cout << start_t.h << ":"  << start_t.m << ":"  << start_t.s << "'" << std::endl;
    if(start_d.y == -1 || start_d.m == -1 || start_d.d == -1 || start_t.h == -1 || start_t.m == -1 || start_t.s == -1 )
    {
        std::stringstream msg;
        msg << "Date and time could not be initialized from filename " << videopath << ", aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }

    /// Load video and template images
    cap.open(videopath); // open a video file
    if(!cap.isOpened())  // check if succeeded
    {
        std::stringstream msg;
        msg << "file " << videopath << " not found or could not be opened";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }
    this->video_w = (int)(cap.get(CAP_PROP_FRAME_WIDTH));
    this->video_h = (int)(cap.get(CAP_PROP_FRAME_HEIGHT));
    this->fps = cap.get(CAP_PROP_FPS);
    this->video_frames = (int)(cap.get(CAP_PROP_FRAME_COUNT));
    if(video_w == 0 || video_h == 0){
        std::stringstream msg;
        msg << "Null dimension(s) for video file " << videopath << " (x=" << video_w << " ,y=" << video_h << "), aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }

    /// Check for existing clock timestamps in tsv file
    //std::string tspath = datapath + videostem + "/tsv/" + videostem + ".tsv";
    std::string tspath = datapath + videostem + ".tsv";
    bool ts_success = this->parseClockTimestampsFile(tspath);
    if(!ts_success){
        std::stringstream msg;
        msg << "Error while parsing clock timestamps file " << tspath << " , aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }
    this->end_clock = this->ts_clock[this->ts_clock.size()-1];
    //this->end_clock = this->start_clock + 1000000000*(float(this->video_frames)/float(this->fps));
    int ts_time_size = this->ts_time.size();
    int ts_clock_size = this->ts_clock.size();
    if(this->video_frames - ts_time_size > 1 || this->video_frames - ts_clock_size > 1){
        std::stringstream msg;
        msg << "Clock timestamps size " << ts_clock_size << " mismatch with video frame size " << this->video_frames << " , aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }

    /// Parse remaining arguments
    std::vector<std::string> csv_list,constraint_list;
    for(int a=2; a<argv.size() ;a++){
        std::string _arg = argv[a];
        std::string ext = getExtension(_arg);
        if(ext=="csv"){
            csv_list.push_back(datapath + _arg);
        }
        else if(ext=="png" || ext =="jpg"){
            std::string template_name = getStemName(_arg);
            template_list.push_back(template_name);
        }
        else{
            std::string _constraint = _arg;

            /// Remove string delimiters
            _constraint.erase (std::remove(_constraint.begin(), _constraint.end(), '\"'), _constraint.end());
            _constraint.erase (std::remove(_constraint.begin(), _constraint.end(), '\''), _constraint.end());
            //_constraint.erase (std::remove(_constraint.begin(), _constraint.end(), '\n'), _constraint.end());

            /// split each constraint in a vector by delimiters like '\n' (and '}'?)
            vector<string> _splitconstraints = splitconstraint (_constraint, '\n');
            for(vector<string>::iterator _splitconstraint = _splitconstraints.begin(); _splitconstraint != _splitconstraints.end(); _splitconstraint++){
                std::cout << "Found constraint: '" << *_splitconstraint << "'"<< std::endl;
                if(! _splitconstraint->empty() ){
                    constraint_list.push_back(*_splitconstraint);
                }
            }
            /*std::cout << "constraint:" << _constraint << std::endl;
            constraint_list.push_back(_constraint);*/
        }
    }

    /// Check for existing csvs
    int stop;
    double time;
    int start = getTickCount();
    double frequency = getTickFrequency();

    //std::map<std::string, std::vector<float> > log;
    //std::vector<int> log_sizes;

    /*for(std::vector<std::string>::iterator _csv = csv_list.begin(); _csv!= csv_list.end();_csv++){
        std::map<std::string, std::vector<float> > _log;
        _log = this->parseCSV(*_csv);
        for(std::map<std::string, std::vector<float> >::iterator _l = _log.begin(); _l!= _log.end();_l++){
            std::map<std::string, std::vector<float> >::iterator _t = log.find(_l->first);
            if(_t==log.end()){
                log[_l->first] = _l->second;
                log_sizes.push_back(_l->second.size());

                std::string _logged_template_path = datapath+_l->first;
                cv::Mat _tmat = cv::imread(_logged_template_path ,1 );
                if(_tmat.empty()){
                    std::cerr << "Template " << _logged_template_path << " could not be opened, aborting..." << std::endl;
                    return 0;
                }
                logged_template_w[_l->first] = _tmat.cols;
                logged_template_h[_l->first] = _tmat.rows;
                _tmat.release();

            }
            else{
                std::cerr << "Template " << _l->first << " logged in " << *_csv << " has already been logged in a previous CSV file, aborting" << std::endl;
                return 0;
            }
        }
    }*/

    /*
    /// Check if image files are available for each template listed in each CSV file
    for(std::vector<std::string>::iterator _csv = csv_list.begin(); _csv!= csv_list.end();_csv++){
        std::map< std::string, std::map<std::string, std::vector<float> > > _log;
        _log = this->parseCSV(*_csv);
        for(std::map<std::string, std::vector<float> >::iterator _l = _log["val"].begin(); _l!= _log["val"].end();_l++){
            std::map<std::string, std::vector<float> >::iterator _t = log_val.find(_l->first);
            if(_t==log_val.end()){
                log_val[_l->first] = _l->second;
                log_sizes.push_back(_l->second.size());

                std::string _logged_template_path = datapath + _l->first + ".png";
                cv::Mat _tmat = cv::imread(_logged_template_path ,1 );
                if(_tmat.empty()){
                    std::cerr << "Template " << _logged_template_path << " could not be opened, aborting..." << std::endl;
                    return 0;
                }
                logged_template_w[_l->first] = _tmat.cols;
                logged_template_h[_l->first] = _tmat.rows;
                _tmat.release();

            }
            else{
                std::cerr << "Template " << _l->first << " logged in " << *_csv << " has already been logged in a previous CSV file, aborting" << std::endl;
                return 0;
            }
        }
        for(std::map<std::string, std::vector<float> >::iterator _l = _log["x"].begin(); _l!= _log["x"].end();_l++){
            log_x[_l->first] = _l->second;
        }
        for(std::map<std::string, std::vector<float> >::iterator _l = _log["y"].begin(); _l!= _log["y"].end();_l++){
            log_y[_l->first] = _l->second;
        }
    }*/
    bool fmf_success = false;
    for(std::vector<std::string>::iterator _csv = csv_list.begin(); _csv != csv_list.end(); _csv++){

        //std::string cv_csvpath = datapath + std::string(argv[1]);
        std::string cv_csvpath = *_csv;
        PCP::CsvConfig* cv_csv;
        try{
            cv_csv = new PCP::CsvConfig(cv_csvpath.c_str());
        }
        catch(...){
            std::stringstream msg;
            msg << "Couldn't open " << cv_csvpath;
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }

        // parse header line
        std::vector<std::string> cv_headers = cv_csv->get_headers();

        std::vector<std::string> _cv_headers;
        for(std::vector<std::string>::iterator cv_header = cv_headers.begin(); cv_header != cv_headers.end(); cv_header++){
            std::string _header(*cv_header);
            _header.erase (std::remove(_header.begin(), _header.end(), '\"'), _header.end());
            _cv_headers.push_back(_header);
        }

        if(_cv_headers[0] == "Frame"){
            std::cout << "Parsing csv file " << cv_csvpath << " to get computer vision events" << std::endl;
            this->parseComputerVisionEvents(cv_csv);
        }
        else if(_cv_headers.size() == 3 && _cv_headers[0] == "h" && _cv_headers[1] == "m" && _cv_headers[2] == "frames"){
            /// If we couldn't successfully parse a timestamps file, let's try to parse a first minute file
            if(!ts_success){
                std::cout << "Parsing csv file " << cv_csvpath << " to get first minute frame count" << std::endl;
                fmf_success = this->parseFirstMinuteFrameFile(cv_csv);
                if(!fmf_success){
                    std::stringstream msg;
                    msg << "Csv file " << cv_csvpath << " didn't contain first minute frame count";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }
            }
        }
        else{
            std::stringstream msg;
            msg << "Csv file " << cv_csvpath << " doesn't contain frame annotations, aborting";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
    }

    /// Ensure that CSV files have the same length
    /*int log_size = 0;
    if(log_sizes.size()>0){
        std::vector<int>::iterator _log_size = log_sizes.begin();
        log_size = *_log_size;
        _log_size++;
        int log_arg = 1;
        for(; _log_size!= log_sizes.end();_log_size++){
            if(*_log_size - log_size > 1 ){
                std::cerr << "Log size mismatch: arg " << log_arg+1 << "=" << *_log_size << " arg " << log_arg  << "=" << log_size << " aborting " << std::endl;
                return 0;
            }
        }
    }*/
    if(log_x.size() > 0){
        std::map<std::string, std::vector<float> >::iterator _log = log_x.begin();
        int log_size = _log->second.size();
        std::string log_name = _log->first;
        _log++;
        for(; _log != log_x.end(); _log++){
            int _log_size = _log->second.size();
            if(_log_size - log_size > 1 ){
                std::stringstream msg;
                msg << "Log size mismatch: size of log " << log_name << " = " << log_size << " while of log " << _log->first  << " = " << _log_size << ", aborting ";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
        }
    }

    /// Check constraints:
    /*bool*/ parse_full_video = false;
    bool needsHookEvents = false;
    bool needsAccessibility = false;
    for(std::vector<std::string>::iterator _constraint = constraint_list.begin(); _constraint!= constraint_list.end();_constraint++){
        std::string _c = *_constraint;

        std::cout << "Parsing constraint: '" << _c << "'" << std::endl;

        std::string _t,_v;
        std::vector<std::string> _vs;

        bool forExtraction = false;
        bool forConversion = false;
        bool forAccessibility = false;
        bool forInputHooks = false;

        /// Check if the constraint has a test statement:
        size_t _a_begin_loc = _c.find("{",0);
        if(_a_begin_loc!=std::string::npos){

            /// Check the test statement:
            size_t _t_end_loc = _c.find("(",0);
            if(_t_end_loc==std::string::npos){
                std::stringstream msg;
                msg << "Constraint '" << _c << "' has a malformed test statement, aborting";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
            _t = _c.substr(0,_t_end_loc);
            _t.erase (std::remove(_t.begin(), _t.end(), ' '), _t.end());

            /// Check if the test is supported:
            bool testSupported = false;
            for(std::vector<std::string>::iterator _statement = supported_extraction_tests.begin(); _statement != supported_extraction_tests.end(); _statement++){
                testSupported |= ( *_statement == _t);
            }
            for(std::vector<std::string>::iterator _statement = supported_conversion_tests.begin(); _statement != supported_conversion_tests.end(); _statement++){
                testSupported |= ( *_statement == _t);
            }
            if(!testSupported){
                std::stringstream msg;
                msg << "Constraint '" << _c << "' has unsupported test statement '" << _t << "', aborting";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
            std::cout << "Constraint '" << _c << "' has a test statement: " << _t << std::endl;

            /// Check the test variable(s):
            size_t _v_end_loc = _c.find(")",_t_end_loc);
            if(_v_end_loc==std::string::npos){
                std::stringstream msg;
                msg << "Constraint '" << _c << "' has a malformed test variable, aborting";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
            _v = _c.substr(_t_end_loc+1,_v_end_loc-(_t_end_loc+1));
            _v.erase (std::remove(_v.begin(), _v.end(), ' '), _v.end());
            size_t _vs_start_loc = 0;
            size_t _vs_end_loc = 0;
            while( _vs_start_loc < _v.size() && _vs_end_loc < _v.size()){
                _vs_end_loc = _v.find(",",_vs_start_loc);
                if(_vs_end_loc==std::string::npos)
                    _vs_end_loc = _v.size();
                std::string __v = _v.substr(_vs_start_loc,_vs_end_loc-(_vs_start_loc));
                __v.erase (std::remove(__v.begin(), __v.end(), ' '), __v.end());
                _vs.push_back(__v);
                _vs_start_loc = _vs_end_loc+1;
            }
            std::cout << "Constraint '" << _c << "' has " << _vs.size() << " test variable(s): ";
            for(std::vector<std::string>::iterator __v = _vs.begin(); __v != _vs.end(); __v++)
                std::cout << *__v << " ";
            std::cout << std::endl;

            //        size_t _a_begin_loc = _c.find("{",_v_end_loc);
            //        if(_a_begin_loc==std::string::npos){
            //            std::cerr << "Constraint '" << _c << "' has a malformed action statement, aborting" << std::endl;
            //            return 0;
            //        }
            _a_begin_loc++;
        }
        else{
            _a_begin_loc = 0;
        }

        /// Check if the constraint has a name assigment:
        std::string _n;
        size_t _n_end_loc = _c.find("=",_a_begin_loc);
        if(_n_end_loc!=std::string::npos){
            _n = _c.substr(_a_begin_loc,_n_end_loc-(_a_begin_loc));
            _n.erase (std::remove(_n.begin(), _n.end(), ' '), _n.end());
            _a_begin_loc = _n_end_loc+1;
            std::cout << "Constraint '" << _c << "' has a name assigment: " << _n << std::endl;
        }

        /// Check the constraint action statement:
        size_t _a_end_loc = _c.find("(",_a_begin_loc);
        if(_a_end_loc==std::string::npos){
            std::stringstream msg;
            msg << "Constraint '" << _c << "' has a malformed action statement, aborting";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        std::string _a = _c.substr(_a_begin_loc,_a_end_loc-(_a_begin_loc));
        _a.erase (std::remove(_a.begin(), _a.end(), ' '), _a.end());

        //	bool actionSupported = false;
        //        for(std::vector<std::string>::iterator _statement = supported_extraction_actions.begin(); _statement != supported_extraction_actions.end(); _statement++){
        //            actionSupported |= ( *_statement == _a);
        //        }
        //        if(!actionSupported){
        //            std::stringstream msg;
        //            msg << "Constraint '" << _c << "' has unsupported action statement " << _a << ", aborting";
        //            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        //        }
        //        std::cout << "Action statement: " << _a << std::endl;

        /// Check statement type:
        forConversion = (std::find(supported_conversion_actions.begin(),supported_conversion_actions.end(),_a) != supported_conversion_actions.end())
                && (std::find(supported_conversion_tests.begin(),supported_conversion_tests.end(),_t) != supported_conversion_tests.end()  || _t.empty());
        forExtraction = (std::find(supported_extraction_actions.begin(),supported_extraction_actions.end(),_a) != supported_extraction_actions.end())
                && (std::find(supported_extraction_tests.begin(),supported_extraction_tests.end(),_t) != supported_extraction_tests.end()  || _t.empty());
        forAccessibility = (std::find(supported_accessibility_actions.begin(),supported_accessibility_actions.end(),_a) != supported_accessibility_actions.end())
                && (std::find(supported_accessibility_tests.begin(),supported_accessibility_tests.end(),_t) != supported_accessibility_tests.end()  || _t.empty());
        forInputHooks = (std::find(supported_input_hook_actions.begin(),supported_input_hook_actions.end(),_a) != supported_input_hook_actions.end())
                && (std::find(supported_input_hook_tests.begin(),supported_input_hook_tests.end(),_t) != supported_input_hook_tests.end()  || _t.empty());
        if(!forConversion && !forExtraction && !forAccessibility && !forInputHooks){
            std::stringstream msg;
            msg << "Constraint '" << _c << "' has unsupported action statement " << _a << ", aborting";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        needsHookEvents |= forInputHooks;
        needsAccessibility |= forAccessibility;

        std::cout << "Constraint '" << _c << "' has an action statement: " << _a << std::endl;

        /// Check action variable(s):
        size_t _av_end_loc = _c.find(")",_a_end_loc);
        if(_av_end_loc==std::string::npos){
            std::stringstream msg;
            msg << "Constraint '" << _c << "' has a malformed action variable, aborting";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        std::string _av = _c.substr(_a_end_loc+1,_av_end_loc-(_a_end_loc+1));
        _av.erase (std::remove(_av.begin(), _av.end(), ' '), _av.end());
        std::vector<std::string> _avs;
        size_t _avs_start_loc = 0;
        size_t _avs_end_loc = 0;
        while( _avs_start_loc < _av.size() && _avs_end_loc < _av.size()){
            _avs_end_loc = _av.find(",",_avs_start_loc);
            if(_avs_end_loc==std::string::npos)
                _avs_end_loc = _av.size();
            std::string __av = _av.substr(_avs_start_loc,_avs_end_loc-(_avs_start_loc));
            __av.erase (std::remove(__av.begin(), __av.end(), ' '), __av.end());
            _avs.push_back(__av);
            _avs_start_loc = _avs_end_loc+1;
        }
        std::cout << "Constraint '" << _c << "' has " << _avs.size() << " action variable(s): ";
        for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++)
            std::cout << *__av << " ";
        std::cout << std::endl;

        /// Process extraction statements:
        if(forExtraction){

            if(_t == "inrect"){
                parse_full_video = true;
            }
            else{
                parse_full_video = false;
            }

            if(_a =="detectText" || _a == "detectNumber" || _a == "detectTime"){
                for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++){
                    text_detect_test[*__av] = _t;
                    text_detect_type[*__av] = _a;

                }
            }

            if(_a =="matchTemplate"){
                for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++){
                    template_matching_test[*__av] = _t;
                    template_matching_type[*__av] = _a;
                }
            }

            if(_a == "template"){
                if(_avs.size()!=6){
                    std::stringstream msg;
                    msg << "Constraint '" << _c << "' with template definition should have 6 parameters instead of "<< _avs.size() << ", aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }
                if(_n.empty()){
                    std::stringstream msg;
                    msg << "Constraint '" << _c << "' should assign the template definition to a template name, as in 'name = template(...)' , aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }

                std::string _videourl = _avs[4];
                std::string _videostem = getStemName(_videourl);

                std::string _videopath = datapath + _videostem + ".mp4";
                /// Load image and template
                cv::VideoCapture _cap;
                _cap.open(_videopath); // open a video file
                if(!_cap.isOpened())  // check if succeeded
                {
                    std::stringstream msg;
                    msg << "File " << _videopath << " not found or could not be opened";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }
                int _video_w = (int)(cap.get(CAP_PROP_FRAME_WIDTH));
                int _video_h = (int)(cap.get(CAP_PROP_FRAME_HEIGHT));
                this->video_frames = (int)(cap.get(CAP_PROP_FRAME_COUNT));
                float _fps = cap.get(CAP_PROP_FPS);
                if(_video_w == 0 || _video_h == 0 || _fps == 0 || this->video_frames == 0){
                    _cap.release();
                    std::stringstream msg;
                    msg << "Null dimension(s) for video file " << _videopath << " (w=" << _video_w << " ,h=" << _video_h << " ,fps=" << _fps<< " ,frames=" << this->video_frames <<"), aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }

                int _x = int( _video_w * atof(_avs[0].c_str()) );
                int _y = ceil( _video_h * atof(_avs[1].c_str()) );
                int _w = int( _video_w * atof(_avs[2].c_str()) );
                int _h = floor( _video_h * atof(_avs[3].c_str()) );
                if(_x == 0 || _y == 0 || _w == 0 || _h == 0){
                    _cap.release();
                    std::stringstream msg;
                    msg << "Null dimension(s) for video file " << _videopath << " (x=" << _x << " ,y=" << _y << " ,w=" << _w << " ,h=" << _h <<"), aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }
                cv::Rect _rect (_x,_y,_w,_h);

                float _t = atof(_avs[5].c_str());

                int _frame_pos = (int)(_t*_fps);
                if(_frame_pos < 0 || _frame_pos > this->video_frames ){
                    _cap.release();
                    std::stringstream msg;
                    msg << "Frame position incompatible with video file " << _videopath << " : asked=" << _frame_pos << " ,max=" << this->video_frames <<"), aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }

                _cap.set(CAP_PROP_POS_FRAMES,_frame_pos);

                cv::Mat _frame;
                bool _frame_read = _cap.read(_frame);

                if(!_frame_read){
                    _cap.release();
                    std::stringstream msg;
                    msg << "Couldn't read the required frame for template "<< _n << " in video file " << _videopath <<" , aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }

                cv::Mat _template = _frame(_rect);

                std::string _templatefile = this->datapath /*+ _videostem + "-"*/ + _n +".png";
                std::cout << _templatefile << std::endl;
                bool _frame_written = cv::imwrite(_templatefile.c_str(),_template);
                if(!_frame_written){
                    _cap.release();
                    std::stringstream msg;
                    msg << "Couldn't save the template "<< _n << " from video file " << _videopath <<" , aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }

                //template_list.push_back(_n); // The next action depending on it will add it to the template list

                _cap.release();
            }

            if(_t.empty()){
                if (_a == "matchTemplate"){
                    for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++){
                        std::vector<std::string>::iterator is_template = std::find(template_list.begin(),template_list.end(),*__av);
                        std::map<std::string, std::vector<float> >::iterator is_logged = log_val.find(*__av);
                        if(is_logged==log_val.end() && is_template==template_list.end()){
                            //std::cerr << "Test variable " << *__v << " is neither part of logged templates nor templates to be extracted, aborting" << std::endl;
                            //return 0;
                            template_list.push_back(*__av);
                        }
                    }
                }
            }
            else if(_t == "inrect"){
                if(_vs.size()!=4){
                    std::stringstream msg;
                    msg << "Constraint '" << _c << "' with inrect test should have 4 parameters instead of "<< _vs.size() << ", aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }
                /*int x = atoi(_vs[0].c_str());
            int y = atoi(_vs[1].c_str());
            int w = atoi(_vs[2].c_str());
            int h = atoi(_vs[3].c_str());
            cv::Rect rect (x,y,w,h);*/

                std::vector<int> rect;
                for(std::vector<std::string>::iterator __v = _vs.begin(); __v != _vs.end(); __v++){
                    rect.push_back( atoi( (*__v).c_str()));
                }

                for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++){
                    inrect_map[*__av] = rect;
                }

                if (_a == "matchTemplate"){
                    for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++){
                        std::vector<std::string>::iterator is_template = std::find(template_list.begin(),template_list.end(),*__av);
                        std::map<std::string, std::vector<float> >::iterator is_logged = log_val.find(*__av);
                        if(is_logged==log_val.end() && is_template==template_list.end()){
                            //std::cerr << "Test variable " << *__v << " is neither part of logged templates nor templates to be extracted, aborting" << std::endl;
                            //return 0;
                            template_list.push_back(*__av);
                        }
                    }
                }
                else if (_a == "detectText" || _a == "detectNumber" || _a == "detectTime"){
                    for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++){
                        text_detect_list.insert(*__av);
                    }
                }
            }
            if(_t == "below" || _t == "rightof"){
                if(_vs.size()!=1){
                    std::stringstream msg;
                    msg <<  "Constraint '" << _c << "' with "<< _t << " test should have 1 parameter instead of "<< _vs.size() << ", aborting";
                    return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
                }
            }
            if(_t == "if" || _t == "between"|| _t == "below"  || _t == "rightof"){

                std::vector<std::string> _template_matching_logged_dep_list;
                std::vector<std::string> _template_matching_new_dep_list;
                std::vector<std::string> _template_matching_dep_list;
                std::vector<std::string> _text_detect_logged_dep_list;
                std::vector<std::string> _text_detect_new_dep_list;
                std::vector<std::string> _text_detect_dep_list;

                for(std::vector<std::string>::iterator __v = _vs.begin(); __v != _vs.end(); __v++){

                    std::map<std::string, std::vector<float> >::iterator is_logged = log_val.find(*__v);

                    bool is_template = (std::find(template_list.begin(),template_list.end(),*__v) != template_list.end());
                    if(is_logged==log_val.end() && !is_template){
                        //std::cerr << "Test variable " << *__v << " is neither part of logged templates nor templates to be extracted, aborting" << std::endl;
                        //return 0;
                        template_list.push_back(*__v);
                        is_template = true;
                    }
                    if(_a == "matchTemplate"){
                        if(is_logged!=log_val.end() ){
                            template_matching_logged_dep_list.push_back(*__v);
                            _template_matching_logged_dep_list.push_back(*__v);
                        }
                        if(is_template){
                            template_matching_new_dep_list.push_back(*__v);
                            _template_matching_new_dep_list.push_back(*__v);
                        }
                        _template_matching_dep_list.push_back(*__v);
                    }
                    else if(_a == "detectText" || _a == "detectNumber" || _a == "detectTime"){
                        if(is_logged!=log_val.end() ){
                            text_detect_logged_dep_list.push_back(*__v);
                            _text_detect_logged_dep_list.push_back(*__v);
                        }
                        if(is_template){
                            text_detect_new_dep_list.push_back(*__v);
                            _text_detect_new_dep_list.push_back(*__v);
                        }
                        _text_detect_dep_list.push_back(*__v);
                    }

                    for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++){
                        if(_a == "matchTemplate"){
                            template_matching_new_dep_map[*__av] = _template_matching_new_dep_list;
                            template_matching_logged_dep_map[*__av]=_template_matching_logged_dep_list;
                            template_matching_dep_map[*__av]=_template_matching_dep_list;

                            std::vector<std::string>::iterator is_template = std::find(template_list.begin(),template_list.end(),*__av);
                            std::map<std::string, std::vector<float> >::iterator is_logged = log_val.find(*__av);
                            if(is_logged==log_val.end() && is_template==template_list.end()){
                                //std::cerr << "Test variable " << *__v << " is neither part of logged templates nor templates to be extracted, aborting" << std::endl;
                                //return 0;
                                template_list.push_back(*__av);
                            }

                        }
                        else if(_a == "detectText" || _a == "detectNumber" || _a == "detectTime"){
                            text_detect_list.insert(*__av);
                            text_detect_new_dep_map[*__av]=_text_detect_new_dep_list;
                            text_detect_logged_dep_map[*__av]=_text_detect_logged_dep_list;
                            text_detect_dep_map[*__av]=_text_detect_dep_list;
                        }
                    }
                }
            }
        }
        /// Process conversion statements:
        else if(forConversion){
            filters.push_back(_n);
            filtering_test[_n] = _t;
            filtering_deps[_n] = _vs;
            filtering_action[_n] = _a;
            filtering_variables[_n] = _avs;
        }
        /// Process accessibility statements:
        else if(forAccessibility){
            ax_annotations.push_back(_n);
            ax_test[_n] = _t;
            ax_deps[_n] = _vs;
            ax_action[_n] = _a;
            ax_variables[_n] = _avs;
        }
        /// Process hook input statements:
        else if(forInputHooks){
            if(_a == "matchKey" && _avs.size() !=1){
                std::stringstream msg;
                msg <<  "Constraint '" << _c << "' with action "<< _a << " should have 1 parameter instead of "<< _avs.size() << ", aborting";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
            else if(_a != "matchKey" && _avs.size() !=0){
                std::stringstream msg;
                msg <<  "Constraint '" << _c << "' with action "<< _a << " should have no parameter instead of "<< _avs.size() << ", aborting";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
            if(std::find(inputhooks.begin(),inputhooks.end(),_n) == inputhooks.end()){
                inputhooks.push_back(_n);
            }
            inputhook_test[_n] = _t;
            inputhook_deps[_n] = _vs;
            inputhook_action[_n] = _a;
            inputhook_variables[_n] = _avs;
        }
    }

    /// Check for existing hook input file
    if(needsHookEvents){
        this->hook_path = datapath + videostem + ".txt";
        bool hook_success = this->checkHookEvents(hook_path);
        if(!hook_success){
            std::stringstream msg;
            msg << "Error while parsing hook input file " << hook_path << " , aborting";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
    }

    /// Load template image whose filename were passed as arguments
    for(std::vector<std::string>::iterator _template = template_list.begin(); _template!= template_list.end();_template++){
        std::string template_name = *_template;
        std::string template_path = datapath + template_name + ".png";
        std::map<std::string, std::vector<float> >::iterator _t = log_val.find(template_name);
        //if(_t==log_val.end()){
        cv::Mat _tmat = cv::imread( template_path,1 );
        if(_tmat.empty()){
            std::stringstream msg;
            msg <<  "Template " << template_name << " could not be opened, aborting...";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        template_w[template_name] = _tmat.cols;
        template_h[template_name] = _tmat.rows;
        templates[template_name] = _tmat.clone();
        /*}
        else{
            std::cerr << "Template " <<template_name << " has already been extracted and logged in a previous CSV file, aborting" << std::endl;
            return 0;
        }*/
    }

    /// Check the processing dependencies of these loaded template images
    for(std::vector<std::string>::iterator _template = template_list.begin(); _template!= template_list.end();_template++){
        std::string _name = *_template;

        std::map<std::string,std::vector<std::string> >::iterator _template_matching_logged = template_matching_logged_dep_map.find(_name);
        std::list<std::string>::iterator _template_matching_new_dep = std::find(template_matching_new_dep_list.begin(),template_matching_new_dep_list.end(), _name);
        std::list<std::string>::iterator _text_detect_new_dep = std::find(text_detect_new_dep_list.begin(),text_detect_new_dep_list.end(),_name);

        if( _template_matching_logged == template_matching_logged_dep_map.end() && _template_matching_new_dep == template_matching_new_dep_list.end() && _text_detect_new_dep == text_detect_new_dep_list.end()){
            std::cout << "Extracting template " << _name << " without constraints" << std::endl;
            template_matching_new_dep_list.push_back(_name);
        }
    }

    //for(std::vector<std::string>::iterator _n = template_list.begin(); _n != template_list.end(); _n++ ){

    /// Prepare one CSV file for the whole export session
    {
        std::string filepath = datapath + videostem;
        for(std::vector<std::string>::iterator _template = template_list.begin(); _template!= template_list.end();_template++){
            std::string template_name = *_template;
            filepath += "+" + template_name;
        }
        for(std::set<std::string>::iterator _text_detection = text_detect_list.begin(); _text_detection!= text_detect_list.end();_text_detection++){
            filepath += "+" + *_text_detection;
        }
        filepath += std::string(".csv");
        file.open(filepath.c_str());
        if(!file.is_open()){
            std::stringstream msg;
            msg << "Couldn't open log file '" << filepath << "'" ;
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        std::cout << "Logging in file '" << filepath << "'" << std::endl;
    }
    file << "\"Frame\"";
    //for(std::vector<std::string>::iterator _template = template_list.begin(); _template!= template_list.end();_template++){
    for(std::vector<std::string>::iterator _template = template_list.begin(); _template!= template_list.end();_template++){
        std::string template_name = *_template;
        file << ",\"" << template_name + ("_x") << "\"";
        file << ",\"" << template_name + ("_y") << "\"";
        file << ",\"" << template_name + ("_val") << "\"";
    }
    for(std::set<std::string>::iterator _text_detection = text_detect_list.begin(); _text_detection!= text_detect_list.end();_text_detection++){
        file << ",\"" << *_text_detection + ("_x") << "\"";
        file << ",\"" << *_text_detection + ("_y") << "\"";
        if( text_detect_type[*_text_detection] == "detectNumber"){
            file << ",\"" << *_text_detection + ("_num") << "\"";
        }
        else if( text_detect_type[*_text_detection] == "detectTime"){
            file << ",\"" << *_text_detection + ("_time") << "\"";
        }
        else{
            file << ",\"" << *_text_detection + ("_txt") << "\"";
        }
    }
    file << std::endl;

    /// Prepare one file per template to extract
    //for(std::vector<std::string>::iterator _template = template_list.begin(); _template!= template_list.end();_template++){
    for(std::vector<std::string>::iterator _template = template_list.begin(); _template!= template_list.end();_template++){
        std::string template_name = *_template;

        std::string filepath = datapath + videostem + "_" + template_name + ".csv";
        csvfile[template_name].open(filepath.c_str());
        if(!csvfile[template_name].is_open()){
            std::stringstream msg;
            msg << "Couldn't open log file '" << filepath << "'";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        std::cout << "Logging in file '" << filepath << "'" << std::endl;

        csvfile[template_name] << "\"Frame\"";
        csvfile[template_name] << ",\"" << template_name + ("_x") << "\"";
        csvfile[template_name] << ",\"" << template_name + ("_y") << "\"";
        csvfile[template_name] << ",\"" << template_name + ("_val") << "\"";
        csvfile[template_name] << std::endl;
    }
    for(std::set<std::string>::iterator _text_detection = text_detect_list.begin(); _text_detection!= text_detect_list.end();_text_detection++){
        std::string filepath = datapath + videostem + "_" + *_text_detection + ".csv";
        csvfile[*_text_detection].open(filepath.c_str());
        if(!csvfile[*_text_detection].is_open()){
            std::stringstream msg;
            msg << "Couldn't open log file '" << filepath << "'";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        std::cout << "Logging in file '" << filepath << "'" << std::endl;

        csvfile[*_text_detection] << "\"Frame\"";
        csvfile[*_text_detection] << ",\"" << *_text_detection + ("_x") << "\"";
        csvfile[*_text_detection] << ",\"" << *_text_detection + ("_y") << "\"";
        if( text_detect_type[*_text_detection] == "detectNumber"){
            csvfile[*_text_detection] << ",\"" << *_text_detection + ("_num") << "\"";
        }
        else if( text_detect_type[*_text_detection] == "detectTime"){
            csvfile[*_text_detection] << ",\"" << *_text_detection + ("_time") << "\"";
        }
        else{
            csvfile[*_text_detection] << ",\"" << *_text_detection + ("_txt") << "\"";
        }
        csvfile[*_text_detection] << std::endl;
    }

    /// Prepare windows to debug extraction by frame
    if(with_gui){
        /// Create windows
        namedWindow( image_window, WINDOW_AUTOSIZE );
        //namedWindow( result_window, WINDOW_AUTOSIZE );
        namedWindow( text_window, WINDOW_KEEPRATIO | WINDOW_NORMAL );

        /// Create Trackbar
        const char* trackbar_label = "Method: \n 0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED";
        //createTrackbar( trackbar_label, image_window, &match_method, max_Trackbar, MatchingMethod );
    }

    //cv::Mat ref_gray, tpl_gray;
    //cv::cvtColor(templ, tpl_gray, COLOR_BGR2GRAY);

    //std::vector<Mat> gray_templates;
    for(std::map<std::string,cv::Mat>::iterator _t = templates.begin(); _t != templates.end(); _t++){
        cv::Mat gray_template;
        cv::cvtColor(_t->second, gray_template, COLOR_BGR2GRAY);
        //gray_template.copyTo(gray_templates[_t->first]);
        gray_templates[_t->first] = gray_template.clone();
    }

    //cv::Mat dst;
    //return true;

    stop = getTickCount();
    time = (double)(stop-start)/frequency;
    std::cout << "Time taken to prepare files and parse constraints: " << time << std::endl;
    stop = start;
    std::cout << std::flush;

    /// Check if the process needs to be aborted
    if(!active){
        std::stringstream msg;
        msg << "Abort requested for video file " << videostem << "), aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }

    /// If a timestamps file has been successfully parsed, accessibility and input hook constraints can be processed before video extraction
    /// Process hook input constraints
    bool hi_pre_success = false;
    if(needsHookEvents){
        this->hook_path = datapath + videostem + ".txt";
        hi_pre_success = !this->parseHookEvents(hook_path,inputhooks,ts_success).empty();
        if(!hi_pre_success){
            std::stringstream msg;
            msg << "Error during hook input processing, aborting";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        stop = getTickCount();
        time = (double)(stop-start)/frequency;
        std::cout << "Time taken to process hook input constraints: " << time << std::endl;
        stop = start;
        std::cout << std::flush;
    }
    bool ax_pre_success = false;
    if(needsAccessibility){
        ax_pre_success = !this->getAccessibilityAnnotations(ax_annotations).empty();
        if(!ax_pre_success){
            std::stringstream msg;
            msg << "Error during accessibilit events processing, aborting";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        stop = getTickCount();
        time = (double)(stop-start)/frequency;
        std::cout << "Time taken to process accessibility constraints: " << time << std::endl;
        stop = start;
        std::cout << std::flush;
    }

    /// Check if the process needs to be aborted
    if(!active){
        std::stringstream msg;
        msg << "Abort requested for video file " << videostem << "), aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }

    /// Process extraction constraints over the video frames
    this->process();
    stop = getTickCount();
    time = (double)(stop-start)/frequency;
    std::cout << "Time taken to process extraction constraints: " << time << std::endl;
    stop = start;
    std::cout << std::flush;

    /// Check if the process needs to be aborted
    if(!active){
        std::stringstream msg;
        msg << "Abort requested for video file " << videostem << "), aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }

    /// If no timestamps file has been successfully parsed, accessibility and input hook constraints must be processed after video extraction
    /// To resynchronize all sources, we determine the amount of video frames from the first minute
    /// (legacy solution before video frames were timestamped)
    if(!ts_success){
        if(first_minute_frames == -1){
            std::stringstream msg;
            msg << "Didn't compute first minute frames, or parse any file indicating it, skipping hook input analysis";
            return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
        }
        else{
            //start_t.h = start_h;
            //start_t.m = start_m;
            start_t.s = 60.0*(float)(fps*60 - first_minute_frames) / (float)(fps*60);

            end_t.s = start_t.h*3600 + start_t.m*60 + start_t.s + this->video_frames/(float)fps;
            end_t.h = floor(end_t.s/3600);
            end_t.m = floor( (end_t.s-3600*end_t.h)/60 );
            end_t.s -= end_t.h*3600 + end_t.m*60;

            std::cout << "Start " << start_t.h << " " << start_t.m << " " << start_t.s << std::endl;
            std::cout << "End " << end_t.h << " " << end_t.m << " " << end_t.s << std::endl;
        }
        stop = getTickCount();
        time = (double)(stop-start)/frequency;
        std::cout << "Time taken to rematch timestamps: " << time << std::endl;
        stop = start;
        std::cout << std::flush;

        /// Process hook input constraints
        if(needsHookEvents && !hi_pre_success){
            this->hook_path = datapath + videostem + ".txt";
            bool hi_post_success = !this->parseHookEvents(hook_path,inputhooks,false).empty();
            if(!hi_post_success){
                std::stringstream msg;
                msg << "Error during hook input processing, aborting";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
            stop = getTickCount();
            time = (double)(stop-start)/frequency;
            std::cout << "Time taken to process hook input constraints: " << time << std::endl;
            stop = start;
            std::cout << std::flush;
        }

        /// Process accessibility constraints
        if(needsAccessibility && !ax_pre_success){
            bool ax_post_success = !this->getAccessibilityAnnotations(ax_annotations).empty();
            if(!ax_post_success){
                std::stringstream msg;
                msg << "Error during accessibility processing, aborting";
                return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            }
            stop = getTickCount();
            time = (double)(stop-start)/frequency;
            std::cout << "Time taken to process accessibility constraints: " << time << std::endl;
            stop = start;
            std::cout << std::flush;
        }
    }

    /// Check if the process needs to be aborted
    if(!active){
        std::stringstream msg;
        msg << "Abort requested for video file " << videostem << "), aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }

    /// Process conversion filterings
    if(filters.empty()){
        std::stringstream msg;
        msg << "done";
        return setStatusAndReturn(/*phase*/"init",/*error*/"", /*success*/msg.str());
    }
    /*bool filtered = this->parseFilterings( filtering_list );
    if(filtered){*/
    bool applied = this->applyFilterings();
    /*}*/
    if(!applied){
        std::stringstream msg;
        msg << "Error during conversion filtering, aborting";
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }
    stop = getTickCount();
    time = (double)(stop-start)/frequency;
    std::cout << "Time taken to apply filters: " << time << std::endl;
    stop = start;

    return setStatusAndReturn(/*phase*/"init",/*error*/"", /*success*/"done");
}

void InspectorWidgetProcessor::abort(){
    this->active = false;
}

void InspectorWidgetProcessor::process(){

    frame = 0;
    //cap.set(CAP_PROP_POS_FRAMES,frame);

    bool parse_condition;

    /*int*/ csv_frame = frame;
    int last_cap_frame = frame;

    this->video_frames = (int)(cap.get(CAP_PROP_FRAME_COUNT));

    //cv::Mat img;

    while(frame < this->video_frames)
    {
        this->status_progress = (float)frame/(float)this->video_frames;
        if(!active){
            std::stringstream msg;
            msg << "Abort requested for video file " << videostem << "), aborting";
            /*return*/ setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
            return;
        }

        int start = getTickCount();
        double frequency = getTickFrequency();

        bool needsTemplateMatching = false;
        bool needsTextDetection = false;

        if(parse_full_video){
            cap.read(img);

            //file << frame;
            this->matchTemplates();
            this->detectText();
            //file << std::endl;
        }
        else{
            bool skip_analysis = true;
            while(csv_frame < this->video_frames && skip_analysis){
                file << frame;

                // Match templates

                for(std::map<std::string,std::string >::iterator _template_matching_test = template_matching_test.begin(); _template_matching_test != template_matching_test.end(); _template_matching_test++){
                    std::cout << "Template matching test '" << _template_matching_test->second << "' empty " << _template_matching_test->second.empty() << " for template '" << _template_matching_test->first << "'" << std::endl;
                    needsTemplateMatching |= _template_matching_test->second.empty();
                }
                for(std::list<std::string>::iterator template_matching_logged_dep = template_matching_logged_dep_list.begin(); template_matching_logged_dep != template_matching_logged_dep_list.end();template_matching_logged_dep++){
                    needsTemplateMatching |= (log_val[*template_matching_logged_dep][csv_frame]>_threshold);
                }
                needsTemplateMatching |= (template_matching_new_dep_list.size()>0);
                needsTemplateMatching |= (text_detect_new_dep_list.size()>0);
                for(std::vector<std::string>::iterator _name = template_list.begin(); _name != template_list.end(); _name++ ){
                    needsTemplateMatching |= (inrect_map.find(*_name)!=inrect_map.end());
                }

                skip_analysis = !needsTemplateMatching;

                bool seek_error = false;
                bool img_read = false;

                if(needsTemplateMatching){
                    if(csv_frame-last_cap_frame>1){
                        bool has_seeked = cap.set(CAP_PROP_POS_FRAMES,csv_frame+1);
                        if(!has_seeked){
                            std::cerr << "Problem seeking to frame " << csv_frame+1 << std::endl;
                            skip_analysis = true;
                            seek_error = true;

                            for(std::map<std::string,cv::Mat>::iterator _template = templates.begin(); _template!=templates.end();_template++){
                                //std::string template_name = getStemName(*_template);
                                std::string template_name = getStemName(_template->first);
                                csvfile[template_name] << frame;
                                csvfile[template_name] << "," << 0;
                                csvfile[template_name] << "," << 0;
                                csvfile[template_name] << "," << 0;
                                csvfile[template_name] << std::endl;
                            }

                            for(int a=0; a<templates.size();a++){
                                file << "," << 0;
                                file << "," << 0;
                                file << "," << 0;
                            }


                        }
                        else{
                            int stop = getTickCount();
                            double time = (double)(stop-start)/frequency;
                            std::cout << "Time taken=" << time << " to skip from frame " << last_cap_frame << " to frame " << frame << std::endl;
                            start=stop;
                        }
                    }
                    img_read = cap.read(img);

                    if(img_read){
                        skip_analysis = false;
                        this->matchTemplates();
                    }

                    //last_cap_frame = csv_frame;
                }

                if(!needsTemplateMatching){

                    for(std::map<std::string,cv::Mat>::iterator _template = templates.begin(); _template!=templates.end();_template++){
                        //std::string template_name = getStemName(*_template);
                        std::string template_name = getStemName(_template->first);
                        csvfile[template_name] << frame;
                        csvfile[template_name] << "," << 0;
                        csvfile[template_name] << "," << 0;
                        csvfile[template_name] << "," << 0;
                        csvfile[template_name] << std::endl;
                    }

                    for(int a=0; a<templates.size();a++){
                        file << "," << 0;
                        file << "," << 0;
                        file << "," << 0;
                    }
                }

                // Detect text

                for(std::list<std::string>::iterator text_detect_logged_dep = text_detect_logged_dep_list.begin(); text_detect_logged_dep != text_detect_logged_dep_list.end();text_detect_logged_dep++){
                    needsTextDetection |= (log_val[*text_detect_logged_dep][csv_frame]>_threshold);
                }
                for(std::list<std::string>::iterator text_detect_new_dep = text_detect_new_dep_list.begin(); text_detect_new_dep != text_detect_new_dep_list.end();text_detect_new_dep++){
                    needsTextDetection |= (template_val[*text_detect_new_dep]>_threshold);
                }
                for(std::set<std::string>::iterator _name = text_detect_list.begin(); _name != text_detect_list.end(); _name++ ){
                    needsTextDetection |= (inrect_map.find(*_name)!=inrect_map.end());
                }

                if(needsTextDetection){
                    if(!img_read){
                        if(csv_frame-last_cap_frame>1){
                            bool has_seeked = cap.set(CAP_PROP_POS_FRAMES,csv_frame+1);
                            if(!has_seeked){
                                std::cerr << "Problem seeking to frame " << csv_frame+1 << std::endl;
                                skip_analysis = true;
                                seek_error = true;
                            }
                            else{
                                int stop = getTickCount();
                                double time = (double)(stop-start)/frequency;
                                std::cout << "Time taken=" << time << " to skip from frame " << last_cap_frame << " to frame " << frame << std::endl;
                                start=stop;
                            }
                        }
                        img_read = cap.read(img);

                    }

                    if(img_read){
                        skip_analysis = false;
                        this->detectText();
                    }



                }

                if(!needsTextDetection || seek_error){
                    for(int a=0; a<text_detect_list.size();a++){
                        file << "," << 0;
                        file << "," << 0;
                        file << "," << " ";
                    }

                    for(std::set<std::string>::iterator _text_detection = text_detect_list.begin(); _text_detection!= text_detect_list.end();_text_detection++){
                        csvfile[*_text_detection] << frame;
                        csvfile[*_text_detection] << "," << 0;
                        csvfile[*_text_detection] << "," << 0;
                        csvfile[*_text_detection] << "," << " ";
                        csvfile[*_text_detection] << std::endl;
                    }
                }

                if(!needsTemplateMatching && !needsTextDetection ){
                    frame++;
                }

                file << std::endl;

                csv_frame++;

            }
            last_cap_frame = frame;
        }

        if(with_gui){
            cv::waitKey(1);
            cv::imshow( image_window, img );
        }

        int stop = getTickCount();

        double time = (double)(stop-start)/frequency;

        std::cout << "Time taken=" << time << " for frame " << frame << " @ " << frames2tc(frame,fps) << std::endl;

        frame++;

    }
    this->status_progress = (frame == this->video_frames ? 1.0 : 0.0);
}



bool InspectorWidgetProcessor::parseFilterings( std::vector<std::string>& filtering_list){
    //filtering_list
    for(std::vector<std::string>::iterator _constraint = filtering_list.begin(); _constraint!= filtering_list.end();_constraint++){
        std::string _c = *_constraint;
        std::cout << "Constraint " << _c << std::endl;

        size_t _t_end_loc = _c.find("(",0);
        if(_t_end_loc==std::string::npos){
            std::cerr << "Constraint '" << _c << "' has a malformed test statement, aborting" << std::endl;
            return 0;
        }
        std::string _t = _c.substr(0,_t_end_loc);
        bool testSupported = false;
        for(std::vector<std::string>::iterator _statement = supported_conversion_tests.begin(); _statement != supported_conversion_tests.end(); _statement++){
            testSupported |= ( *_statement == _t);
        }
        if(!testSupported){
            std::cerr << "Constraint '" << _c << "' has unsupported test statement '" << _t << "', aborting" << std::endl;
            return 0;
        }
        std::cout << "Test statement: " << _t << std::endl;

        size_t _v_end_loc = _c.find(")",_t_end_loc);
        if(_v_end_loc==std::string::npos){
            std::cerr << "Constraint '" << _c << "' has a malformed test variable, aborting" << std::endl;
            return 0;
        }
        std::string _v = _c.substr(_t_end_loc+1,_v_end_loc-(_t_end_loc+1));
        std::vector<std::string> _vs;
        size_t _vs_start_loc = 0;
        size_t _vs_end_loc = 0;
        while( _vs_start_loc < _v.size() && _vs_end_loc < _v.size()){
            _vs_end_loc = _v.find(",",_vs_start_loc);
            if(_vs_end_loc==std::string::npos)
                _vs_end_loc = _v.size();
            std::string __v = _v.substr(_vs_start_loc,_vs_end_loc-(_vs_start_loc));
            _vs.push_back(__v);
            _vs_start_loc = _vs_end_loc+1;
        }
        std::cout << _vs.size() << " test variable(s): ";
        for(std::vector<std::string>::iterator __v = _vs.begin(); __v != _vs.end(); __v++)
            std::cout << *__v << " ";
        std::cout << std::endl;

        size_t _n_begin_loc = _c.find("{",_v_end_loc);
        if(_n_begin_loc==std::string::npos){
            std::cerr << "Constraint '" << _c << "' has a malformed filter name, aborting" << std::endl;
            return 0;
        }

        size_t _n_end_loc = _c.find("=",_n_begin_loc);
        if(_n_end_loc==std::string::npos){
            std::cerr << "Constraint '" << _c << "' has a malformed action statement, aborting" << std::endl;
            return 0;
        }
        std::string _n = _c.substr(_n_begin_loc+1,_n_end_loc-_n_begin_loc-1);
        std::cout << "Filter name: " << _n << std::endl;

        size_t _a_end_loc = _c.find("(",_n_end_loc);
        if(_a_end_loc==std::string::npos){
            std::cerr << "Constraint '" << _c << "' has a malformed action statement, aborting" << std::endl;
            return 0;
        }

        std::string _a = _c.substr(_n_end_loc+1,_a_end_loc-(_n_end_loc+1));
        bool actionSupported = false;
        for(std::vector<std::string>::iterator _statement = supported_conversion_actions.begin(); _statement != supported_conversion_actions.end(); _statement++){
            actionSupported |= ( *_statement == _a);
        }
        if(!actionSupported){
            std::cerr << "Constraint '" << _c << "' has unsupported test statement " << _a << ", aborting" << std::endl;
            return 0;
        }
        std::cout << "Action statement: " << _a << std::endl;

        size_t _av_end_loc = _c.find(")}",_a_end_loc);
        if(_av_end_loc==std::string::npos){
            std::cerr << "Constraint '" << _c << "' has a malformed action variable, aborting" << std::endl;
            return 0;
        }
        std::string _av = _c.substr(_a_end_loc+1,_av_end_loc-(_a_end_loc+1));
        std::vector<std::string> _avs;
        size_t _avs_start_loc = 0;
        size_t _avs_end_loc = 0;
        while( _avs_start_loc < _av.size() && _avs_end_loc < _av.size()){
            _avs_end_loc = _av.find(",",_avs_start_loc);
            if(_avs_end_loc==std::string::npos)
                _avs_end_loc = _av.size();
            std::string __av = _av.substr(_avs_start_loc,_avs_end_loc-(_avs_start_loc));
            _avs.push_back(__av);
            _avs_start_loc = _avs_end_loc+1;
        }
        std::cout << _avs.size() << " action variable(s): ";
        for(std::vector<std::string>::iterator __av = _avs.begin(); __av != _avs.end(); __av++)
            std::cout << *__av << " ";
        std::cout << std::endl;

        ///
        filters.push_back(_n);
        filtering_test[_n] = _t;
        filtering_deps[_n] = _vs;
        filtering_action[_n] = _a;
        filtering_variables[_n] = _avs;
    }

    return true;
}

bool InspectorWidgetProcessor::applyFilterings(){

    for(std::vector<std::string>::iterator filter = filters.begin(); filter != filters.end(); filter++){

        int stop;
        double time;
        int start = getTickCount();
        double frequency = getTickFrequency();

        /// Test if we need to parse hook events:
        bool needsHookEvents = false;
        std::vector<std::string> _filtering_deps = filtering_deps[*filter];
        for(std::vector<std::string>::iterator _filtering_dep = _filtering_deps.begin(); _filtering_dep != _filtering_deps.end(); _filtering_dep++){
            std::map<std::string, std::vector<float> >::iterator _log_val = log_val.find(*_filtering_dep);
            std::map<std::string, std::vector<std::string> >::iterator _log_txt = log_txt.find(*_filtering_dep);
            if(_log_val == log_val.end() && _log_txt == log_txt.end()){
                needsHookEvents = true;
                break;
            }
        }

        std::vector<std::string> _filtering_variables = filtering_variables[*filter];

        /*if( filtering_test[*filter] == "during"){

        }*/

        if(!needsHookEvents){

            if( filtering_action[*filter] == "nestByLastVariable" ){
                std::vector<std::string> _filtering_variables = filtering_variables[*filter];
                std::string _filtering_dep = filtering_deps[*filter][0];
                int frames = log_val[_filtering_dep].size();

                std::ofstream segmentfile;
                std::string segmentfilepath = datapath + videostem + std::string("_") + *filter + std::string(".csv");
                segmentfile.open(segmentfilepath.c_str());

                if(!segmentfile.is_open()){
                    std::cerr << " Couldn't open file " << segmentfilepath << ", aborting..." << std::endl;
                    return false;
                }

                std::map<std::string,int> _seg_id;
                std::map<std::string,int> _seg_start;
                std::map<std::string,int> _seg_end;
                std::map<std::string,int> _val_is_txt;
                std::map<std::string,float> _prev_val;
                std::map<std::string,std::string> _prev_txt;

                std::map<std::string,std::vector<int> > _line_start_ids,_line_end_ids;
                std::map<std::string,std::vector<std::string> > _line_start_vals,_line_end_vals;
                std::map<std::string,std::vector<int> > _line_txt_ids;
                std::map<std::string,std::vector<std::string> > _line_txt_vals;

                for(std::vector<std::string>::iterator _filtering_variable = _filtering_variables.begin(); _filtering_variable != _filtering_variables.end(); _filtering_variable++ ){
                    segmentfile << "\""<< *_filtering_variable << "Start" << "\",";
                    //segmentfile << "\""<< *_filtering_variable << "End" << "\",";
                    _seg_id[*_filtering_variable] = 0;
                    _seg_start[*_filtering_variable] = 0;
                    _val_is_txt[*_filtering_variable] = (log_val.find(*_filtering_variable) == log_val.end());
                    if(_val_is_txt[*_filtering_variable]){
                        _prev_txt[*_filtering_variable] = log_txt[*_filtering_variable][0];
                        segmentfile << "\""<< *_filtering_variable << "Value" << "\",";
                    }
                    else{
                        _prev_val[*_filtering_variable] = log_val[*_filtering_variable][0]>_threshold?1:0;
                    }
                }
                segmentfile << "\"Duration\",\"tcin\",\"tcout\"" << std::endl;

                int seg_start = 0;

                std::string _nest_variable = _filtering_variables.back();

                for(int f=1;f<frames;f++){

                    std::map<std::string,float> _cur_val;
                    std::map<std::string,std::string> _cur_txt;

                    bool save_segment = false;

                    for(std::vector<std::string>::iterator _filtering_variable = _filtering_variables.begin(); _filtering_variable != _filtering_variables.end(); _filtering_variable++ ){
                        if(_val_is_txt[*_filtering_variable]){
                            _cur_txt[*_filtering_variable] = log_txt[*_filtering_variable][f];
                            if ( (( _prev_txt[*_filtering_variable] != _cur_txt[*_filtering_variable]) || f==1) && _cur_txt[*_filtering_variable]!=" " && _cur_txt[*_filtering_variable]!=""){
                                // New segment
                                //seg_start = f;
                                _seg_id[*_filtering_variable] +=1;
                                _seg_start[*_filtering_variable] = f;

                                _line_start_vals[*_filtering_variable].push_back( frames2dtc( f , this->fps, this->start_d, this->start_t) );
                                _line_txt_vals[*_filtering_variable].push_back( _cur_txt[*_filtering_variable] );
                            }
                            else if ((_prev_txt[*_filtering_variable] != _cur_txt[*_filtering_variable] || f==frames-1) && _prev_txt[*_filtering_variable]!=" " && _prev_txt[*_filtering_variable]!=""){
                                // End segment
                                //save_segment = true;
                                _seg_end[*_filtering_variable] = f;

                                _line_end_vals[*_filtering_variable].push_back( frames2dtc( f , this->fps, this->start_d, this->start_t) );
                            }
                        }
                        else{
                            _cur_val[*_filtering_variable] = log_val[*_filtering_variable][f]>_threshold?1:0;
                            if ((_prev_val[*_filtering_variable]== 0 || f==1) && _cur_val[*_filtering_variable] == 1){
                                // New segment
                                //seg_start = f;
                                _seg_id[*_filtering_variable] +=1;
                                _seg_start[*_filtering_variable] = f;

                                _line_start_vals[*_filtering_variable].push_back( frames2dtc( f , this->fps, this->start_d, this->start_t) );
                            }
                            else if (_prev_val[*_filtering_variable] == 1 && (_cur_val[*_filtering_variable] == 0 || f==frames-1)){
                                // End segment
                                //save_segment = true;
                                _seg_end[*_filtering_variable] = f;

                                _line_end_vals[*_filtering_variable].push_back( frames2dtc( f , this->fps, this->start_d, this->start_t) );
                            }
                        }


                    }

                    if(_val_is_txt[_nest_variable]){
                        if ((_prev_txt[_nest_variable] != _cur_txt[_nest_variable] || f==1) && _cur_txt[_nest_variable]!=" " && _cur_txt[_nest_variable]!=""){
                            // New segment
                            seg_start = f;
                            _seg_start[_nest_variable] = f;
                        }
                        else if ((_prev_txt[_nest_variable] != _cur_txt[_nest_variable] || f==frames-1) && _prev_txt[_nest_variable]!=" " && _prev_txt[_nest_variable]!=""){
                            // End segment
                            save_segment = true;
                            _seg_end[_nest_variable] = f;
                        }
                    }
                    else{
                        if ((_prev_val[_nest_variable]== 0 || f==1) && _cur_val[_nest_variable] == 1){
                            // New segment
                            seg_start = f;
                            _seg_start[_nest_variable] = f;
                        }
                        else  if (_prev_val[_nest_variable] == 1 && (_cur_val[_nest_variable] == 0 || f==frames-1)){
                            // End segment
                            save_segment = true;
                            _seg_end[_nest_variable] = f;
                        }
                    }

                    for(std::vector<std::string>::iterator _filtering_variable = _filtering_variables.begin(); _filtering_variable != _filtering_variables.end(); _filtering_variable++ ){
                        if(save_segment){
                            segmentfile << frames2dtc( _seg_start[*_filtering_variable] , this->fps, this->start_d, this->start_t) << ",";

                            _line_start_ids[*_filtering_variable].push_back(  _seg_id[*_filtering_variable]-1 );
                            _line_end_ids[*_filtering_variable].push_back(  _seg_id[*_filtering_variable]-1 );

                        }
                        if(_val_is_txt[*_filtering_variable]){
                            if(save_segment){
                                segmentfile << _prev_txt[*_filtering_variable] << ",";

                                _line_txt_ids[*_filtering_variable].push_back(  _seg_id[*_filtering_variable]-1 );

                            }
                            _prev_txt[*_filtering_variable] = _cur_txt[*_filtering_variable];
                        }
                        else{
                            if(save_segment){
                                //segmentfile << _seg_id[*_filtering_variable] << ",";
                            }
                            _prev_val[*_filtering_variable] = _cur_val[*_filtering_variable];
                        }
                    }

                    if(save_segment){
                        segmentfile << (float)(f-seg_start)/(float)(fps) << "," << frames2tc(seg_start, this->fps) << "," << frames2tc(f, this->fps)  << std::endl;
                    }

                    save_segment = false;

                }



                segmentfile.close();

                /*std::ofstream segmentfile;*/
                /*std::string*/ segmentfilepath = datapath + videostem + std::string("_") + *filter + "StartEnd" + std::string(".csv");
                segmentfile.open(segmentfilepath.c_str());

                if(!segmentfile.is_open()){
                    std::cerr << " Couldn't open file " << segmentfilepath << ", aborting..." << std::endl;
                    return false;
                }

                for(std::vector<std::string>::iterator _filtering_variable = _filtering_variables.begin(); _filtering_variable != _filtering_variables.end(); _filtering_variable++ ){
                    segmentfile << "\""<< *_filtering_variable << "Start" << "\",";
                    segmentfile << "\""<< *_filtering_variable << "End" << "\",";
                    if(_val_is_txt[*_filtering_variable]){
                        segmentfile << "\""<< *_filtering_variable << "Value" << "\",";
                    }
                }
                segmentfile << " " << std::endl;

                int lines = _line_start_ids[_nest_variable].size();

                for(int f=0;f<lines;f++){
                    for(std::vector<std::string>::iterator _filtering_variable = _filtering_variables.begin(); _filtering_variable != _filtering_variables.end(); _filtering_variable++ ){
                        segmentfile << _line_start_vals[*_filtering_variable][ _line_start_ids[*_filtering_variable][f] ]  << ",";
                        segmentfile << _line_end_vals[*_filtering_variable][ _line_end_ids[*_filtering_variable][f] ]  << ",";
                        if(_val_is_txt[*_filtering_variable]){
                            segmentfile << _line_txt_vals[*_filtering_variable][ _line_txt_ids[*_filtering_variable][f] ]  << ",";
                        }
                    }
                    segmentfile << ", " << std::endl;
                }

                segmentfile.close();



            }
            else if( filtering_action[*filter] == "eval" ){

                std::string _filtering_variable = filtering_variables[*filter][0];
                std::string _filtering_dep = filtering_deps[*filter][0];
                std::vector<float> _val;
                int frames = log_val[_filtering_dep].size();
                std::vector<std::string> _txt;
                std::vector<float> _x(frames,0.0);
                std::vector<float> _y(frames,0.0);

                int dep_seg_start = 0;
                bool segment_on = false;

                StringBuffer sb_s,sb_o;
                PrettyWriter<StringBuffer>* w_s = new  PrettyWriter<StringBuffer> (sb_s);
                //PrettyWriter<StringBuffer>* w_o = new  PrettyWriter<StringBuffer> (sb_o);

                header(*w_s);

                float _prev_dep_val = (log_val[_filtering_dep][0] >_threshold)?1:0;
                float _prev_var_val(0.0);
                this->current_frame = 0;
                try{
                    InspectorWidgetProcessorCommandParser::stacks _parser_stacks;
                    pegtl::parse< InspectorWidgetProcessorCommandParser::grammar, InspectorWidgetProcessorCommandParser::action >( _filtering_variable, _filtering_variable, *parser_operators, _parser_stacks );
                    std::string __val_str = _parser_stacks.get(*parser_operators);
                    //std::cout << "Filter " <<  _filtering_variable << " is " << __val_str << std::endl;
                    if(__val_str == "True"){
                        _prev_var_val = 1.0;
                    }
                }
                catch(...){
                    std::cerr << "Couldn't' parse '" << _filtering_variable <<  "'" << std::endl;
                    return 0; //exit(0);
                }

                int _prev_dep_seg_start = 0;

                for(int f=1;f<frames;f++){
                    float _cur_var_val = 0;

                    float _cur_dep_val = (log_val[_filtering_dep][f] >_threshold)?1:0;

                    if(_cur_dep_val == 1){
                        this->current_frame = f;

                        try{
                            InspectorWidgetProcessorCommandParser::stacks _parser_stacks;
                            pegtl::parse< InspectorWidgetProcessorCommandParser::grammar, InspectorWidgetProcessorCommandParser::action >( _filtering_variable, _filtering_variable, *parser_operators, _parser_stacks );
                            std::string __val_str = _parser_stacks.get(*parser_operators);
                            //std::cout << "Filter " <<  _filtering_variable << " is " << __val_str << std::endl;
                            if(__val_str == "True"){
                                _cur_var_val = 1.0;
                            }
                        }
                        catch(...){
                            std::cerr << "Couldn't' parse '" << _filtering_variable <<  "'" << std::endl;
                            return 0; //exit(1);
                        }
                    }

                    if(_prev_var_val == 0 && _cur_var_val == 1 /*&& _cur_dep_val == 1*/){
                        dep_seg_start = f;
                        segment_on = true;
                    }
                    else if( (_prev_dep_val == 1 && _cur_dep_val == 0 && _prev_var_val == 1) || (_prev_var_val == 1 && _cur_var_val == 0 && _prev_dep_val == 1) || (f==frames-1 && _cur_dep_val == 1) || (f==frames-1 && _cur_var_val == 1)){
                        if(_prev_dep_seg_start == dep_seg_start ){
                            std::cerr << "Duplicate segment start " << dep_seg_start << " @ "<< frames2tc(dep_seg_start, this->fps) << ", aborting" << std::endl;
                            return 0; //exit(0);
                        }

                        std::stringstream label;
                        label << *filter;
                        segment(*w_s, dep_seg_start,f/*-1*/, this->fps, label.str());
                        //std::cout << "Frame start " << dep_seg_start << " @ "<< frames2tc(dep_seg_start); // << std::endl;
                        //std::cout << " frame end " << f << " @ "<< frames2tc(f) << std::endl;
                        segment_on = false;
                        _prev_dep_seg_start = dep_seg_start;
                    }

                    _val.push_back(_cur_var_val);

                    _prev_dep_val = _cur_dep_val;
                    _prev_var_val = _cur_var_val;

                }

                log_x[*filter] = _x;
                log_y[*filter] = _y;
                log_val[*filter] = _val;

                segmentfooter(*w_s,*filter,"combined",this->video_frames, this->fps);

                std::ofstream segmentfile;
                std::string segmentfilepath = datapath + videostem + std::string("-") + *filter + std::string("-segments.json");
                segmentfile.open(segmentfilepath.c_str());

                if(segmentfile.is_open()){
                    segmentfile << sb_s.GetString();
                    segmentfile.close();
                }

            }
            else if( filtering_action[*filter] == "matchFirstValueOf" ){

                std::string _filtering_dep = _filtering_deps[0];
                std::string _filtering_variable = _filtering_variables[0];
                int frames = log_val[_filtering_dep].size();
                std::vector<float> _val;
                std::vector<std::string> _txt;
                std::vector<float> _x(frames,0.0);
                std::vector<float> _y(frames,0.0);

                bool isVarFloat = (log_val.find(_filtering_variable) != log_val.end());
                bool isVarTxt = (log_txt.find(_filtering_variable) != log_txt.end());

                StringBuffer sb_s,sb_o;
                PrettyWriter<StringBuffer>* w_s = new  PrettyWriter<StringBuffer> (sb_s);
                //PrettyWriter<StringBuffer>* w_o = new  PrettyWriter<StringBuffer> (sb_o);

                header(*w_s);

                int var_f = 0;

                float _prev_dep_val = (log_val[_filtering_dep][0] >_threshold)?1:0;
                std::string _prev_var_txt("");
                float _prev_var_val(0.0);
                if(isVarFloat) _prev_var_val = (log_val[_filtering_variable][0] >_threshold)?1:0;
                if(isVarTxt) _prev_var_txt = log_txt[_filtering_variable][0];

                int dep_seg_start = 0;
                bool segment_on = false;

                int var_size = 0;
                if(isVarFloat) var_size = log_val[_filtering_variable].size();
                if(isVarTxt) var_size = log_txt[_filtering_variable].size();

                int dep_f = 1;
                while(dep_f<log_val[_filtering_dep].size() && var_f < var_size){

                    float _cur_dep_val = (log_val[_filtering_dep][dep_f] >_threshold)?1:0;

                    if( _prev_dep_val == 0 && _cur_dep_val == 1){
                        dep_seg_start = dep_f;
                        segment_on = true;
                    }
                    else if( _prev_dep_val == 1 && _cur_dep_val == 0){
                        var_f = dep_seg_start;

                        float _cur_var_val(0.0);
                        std::string _cur_var_txt(" ");

                        while(var_f < dep_f && var_f < var_size){
                            if(isVarFloat){
                                _cur_var_val = (log_val[_filtering_variable][var_f] >_threshold)?1:0;
                                if(_cur_var_val == 1){
                                    break;
                                }
                            }
                            else if(isVarTxt){

                                _cur_var_txt = log_txt[_filtering_variable][var_f];
                                if(_cur_var_txt != " "){
                                    break;
                                }
                            }
                            var_f++;
                        }

                        for(int _f=dep_seg_start; _f < dep_f; _f++){
                            if(isVarFloat){
                                _val.push_back(_cur_var_val);
                            }
                            else if(isVarTxt){
                                _txt.push_back(_cur_var_txt);
                            }

                        }
                        std::stringstream label;
                        if(isVarFloat) label << _cur_var_val;
                        if(isVarTxt) label << _cur_var_txt;

                        segment(*w_s, dep_seg_start,dep_f/*-1*/,this->fps,label.str());
                        segment_on = false;

                    }
                    else{
                        if( _cur_dep_val == 0 && _prev_dep_val == 0){
                            if(isVarFloat){
                                _val.push_back(0.0);
                            }
                            else if(isVarTxt){
                                _txt.push_back(" ");
                            }
                        }
                        dep_f++;
                    }

                    _prev_dep_val = _cur_dep_val;
                }

                log_x[*filter] = _x;
                log_y[*filter] = _y;
                if(isVarFloat) log_val[*filter] = _val;
                if(isVarTxt) log_txt[*filter] = _txt;

                segmentfooter(*w_s,*filter,"combined",video_frames, this->fps);

                std::ofstream segmentfile;
                std::string segmentfilepath = datapath + videostem + std::string("-") + *filter + std::string("-segments.json");
                segmentfile.open(segmentfilepath.c_str());

                if(segmentfile.is_open()){
                    segmentfile << sb_s.GetString();
                    segmentfile.close();
                }
            }else if( filtering_action[*filter] == "triggerBySegmentsOf" ){
                std::string _filtering_dep = _filtering_deps[0];
                std::string _filtering_variable = _filtering_variables[0];
                int frames = log_val[_filtering_dep].size();
                std::vector<float> _val;
                std::vector<float> _x(frames,0.0);
                std::vector<float> _y(frames,0.0);
                float _prev_dep_val = (log_val[_filtering_dep][0] >_threshold)?1:0;
                float _prev_var_val = (log_val[_filtering_variable][0] >_threshold)?1:0;
                float _seg_start = 0;
                float __val = 0.0;
                bool segment_on = false;

                StringBuffer sb_s,sb_o;
                PrettyWriter<StringBuffer>* w_s = new  PrettyWriter<StringBuffer> (sb_s);
                //PrettyWriter<StringBuffer>* w_o = new  PrettyWriter<StringBuffer> (sb_o);

                header(*w_s);

                int _segments = 0;

                for(int f=1;f<log_val[_filtering_dep].size();f++){

                    float _cur_dep_val = (log_val[_filtering_dep][f]>_threshold)?1:0;
                    float _cur_var_val = (log_val[_filtering_variable][f]>_threshold)?1:0;

                    if( _cur_dep_val == 1 && _cur_var_val == 1 && _prev_var_val == 0){
                        // Segment starts
                        __val = 1.0;
                        if(segment_on){
                            std::stringstream _label;
                            _label << *filter << " #" << ++_segments;
                            segment(*w_s, _seg_start,f/*-1*/, this->fps, _label.str());
                        }
                        _seg_start = f;
                        segment_on = true;
                    }
                    else if(_cur_dep_val == 0 && _prev_dep_val == 1){
                        // Segment ends
                        if(segment_on){
                            std::stringstream _label;
                            _label << *filter << " #" << ++_segments;
                            segment(*w_s, _seg_start,f, this->fps, _label.str());
                            segment_on = false;
                        }
                        __val = 0.0;
                    }

                    if(_seg_start == f){ // To have 0.0 at the end frame of each segment
                        __val = 0.0;
                    }

                    _val.push_back(__val);

                    if(_seg_start == f){ // To have 0.0 at the end frame of each segment
                        __val = 1.0;
                    }

                    _prev_dep_val = _cur_dep_val;
                    _prev_var_val = _cur_var_val;

                }

                log_x[*filter] = _x;
                log_y[*filter] = _y;
                log_val[*filter] = _val;

                segmentfooter(*w_s,*filter,"combined",this->video_frames, this->fps);

                std::ofstream segmentfile;
                std::string segmentfilepath = datapath + videostem + std::string("-") + *filter + std::string("-segments.json");
                segmentfile.open(segmentfilepath.c_str());

                if(segmentfile.is_open()){
                    segmentfile << sb_s.GetString();
                    segmentfile.close();
                }

            }

        }

        stop = getTickCount();
        time = (double)(stop-start)/frequency;
        std::cout << "Time taken to parse csv and save jsons: " << time << std::endl;

    }

    return 0;
}

bool InspectorWidgetProcessor::parseClockTimestampsFile(std::string _path){

    // instantiate parser
    PCP::CsvConfig* tsv_config;
    try{
        tsv_config = new PCP::CsvConfig(_path.c_str(), false, '\t');
    }
    catch(...){
        std::stringstream msg;
        msg <<  "Couldn't open " << _path;
        return setStatusAndReturn(/*phase*/"init",/*error*/msg.str(), /*success*/"");
    }
    PCP::PartialCsvParser parser(*tsv_config);

    bool first_row = true;

    // parse & print body lines
    std::vector<std::string> row;
    while (!(row = parser.get_row()).empty()) {
        if(row.size() == 3){
            if(first_row){
                ts_time.clear();
                ts_clock.clear();
                start_clock = std::stoull(row[2]);
                first_row = false;
            }
            ts_time[atoi(row[0].c_str())] = std::stoull(row[1]);
            ts_clock[atoi(row[0].c_str())] = std::stoull(row[2]);
        }
    }
    return true;
}

bool InspectorWidgetProcessor::parseFirstMinuteFrameFile(PCP::CsvConfig* cv_csv){
    if(!cv_csv){
        return 0;
    }

    // instantiate parser
    PCP::PartialCsvParser parser(*cv_csv);  // parses whole body of CSV without range options.

    // parse & print body lines
    std::vector<std::string> row;
    row = parser.get_row();
    if(row.size() == 3){
        start_h = atoi(row[0].c_str());
        start_m = atoi(row[1].c_str());
        first_minute_frames = atoi(row[2].c_str());
        return true;
    }
    return false;
}

bool InspectorWidgetProcessor::checkHookEvents(std::string hook_path){
    std::ifstream hook_txt;
    hook_txt.open(hook_path.c_str());
    if(!hook_txt.is_open()){
        std::stringstream msg;
        msg <<  "Couldn't open hook input file " << hook_path;
        return setStatusAndReturn(/*phase*/"checkHookEvents",/*error*/msg.str(), /*success*/"");
    }
    std::string line;
    getline(hook_txt,line);
    if(line.empty()){
        std::stringstream msg;
        msg <<  "Empty line in hook input file " << hook_path;
        setStatusAndReturn(/*phase*/"checkHookEvents",/*error*/msg.str(), /*success*/"");
        return false;
    }
    std::string id("id=");
    size_t idpos = line.find(id);
    if(idpos == string::npos){
        std::stringstream msg;
        msg <<  "No id element in hook input file " << hook_path;
        setStatusAndReturn(/*phase*/"checkHookEvents",/*error*/msg.str(), /*success*/"");
        return false;
    }
    std::string when("when=");
    size_t whenpos = line.find(when);
    if(whenpos == string::npos){
        std::stringstream msg;
        msg <<  "No timestamp element in hook input file " << hook_path;
        setStatusAndReturn(/*phase*/"checkHookEvents",/*error*/msg.str(), /*success*/"");
        return false;
    }
    hook_txt.close();
    return true;
}

std::vector<std::string> InspectorWidgetProcessor::parseHookEvents(std::string hook_path, std::vector<std::string> names, bool using_clocktime){
    std::vector<std::string> annotations;
    std::ifstream hook_txt;
    hook_txt.open(hook_path.c_str());
    if(!hook_txt.is_open()){
        std::stringstream msg;
        msg <<  "Couldn't open hook input file " << hook_path;
        setStatusAndReturn(/*phase*/"parseHookEvents",/*error*/msg.str(), /*success*/"");
        return annotations;
    }
    if(!hook_txt.is_open()){
        std::stringstream msg;
        std::cerr <<  "Couldn't open hook input file " << hook_path;
        setStatusAndReturn(/*phase*/"parseHookEvents",/*error*/msg.str(), /*success*/"");
        return annotations;
    }
    hook_txt.seekg(ios::beg);

    /// Check if names are listed in registered input hook definitions in the pipeline
    for(std::vector<std::string>::iterator name = names.begin(); name != names.end();name++ ){
        if( std::find(inputhooks.begin(), inputhooks.end(), *name) == inputhooks.end()){
            std::stringstream msg;
            std::cerr <<  *name << "is not part of registered input hook definitions in the pipeline, aborting";
            setStatusAndReturn(/*phase*/"parseHookEvents",/*error*/msg.str(), /*success*/"");
            return annotations;
        }
    }

    /// Initialze maps of active actions and their related annotation name
    std::map<std::string,bool> isActionActive;
    std::map<std::string,std::string> annotationName;
    for(std::vector<std::string>::iterator action = inputhooks.begin(); action != inputhooks.end();action++ ){
        isActionActive[*action] = false;
        annotationName[*action] = "";
    }

    int stop;
    double time;
    int start = getTickCount();
    double frequency = getTickFrequency();

    std::map<std::string,StringBuffer> s_o,s_s;
    std::map<std::string,PrettyWriter<StringBuffer>* > w_o,w_s;

    for(std::vector<std::string>::iterator name = names.begin(); name != names.end();name++ ){
        s_o[*name] = StringBuffer();
        s_s[*name] = StringBuffer();
        PrettyWriter<StringBuffer>* _o = new PrettyWriter<StringBuffer>(s_o[*name]);
        PrettyWriter<StringBuffer>* _s = new PrettyWriter<StringBuffer>(s_s[*name]);
        w_o[*name] = _o;
        w_s[*name] = _s;
        header(*_o);
        header(*_s);
        std::string action = inputhook_action[*name];
        isActionActive[action] = true;
        annotationName[action] = *name;
    }

    // parse & print body lines
    std::string line;
    int r =0;
    std::string ts;
    double ts_start,ts_end;
    if(using_clocktime){
        ts = "clock";
        ts_start = double(this->start_clock )/1000000000.0;
        ts_end = double(this->end_clock )/1000000000.0;
    }
    else {
        ts = "when";
        ts_start = start_t.h*3600 + start_t.m*60 + start_t.s;
        ts_end = end_t.h*3600 + end_t.m*60 + end_t.s;
    }
    bool prev_event_is_mouse = false;
    bool prev_event_is_keyboard = false;
    std::string word;
    float wordin(0.0), wordout(0.0);
    uint16_t keychar_int;
    std::string keychar_str;
    uint16_t keycode_int = 0;
    uint16_t rawcode_int = 0;
    float keychar_time(0.0),keycode_time(0.0),rawcode_time(0.0);
    float __x(0.0),__y(0.0);
    std::vector<float> _x,_y;
    std::vector<std::string> _words;
    bool keycode_canformword = false;
    std::string keycode_modifier_string = "";
    bool keychar_canformword = false;

    int _frame = 0;

    while (getline(hook_txt,line)) {
        std::vector<std::string> row = splitconstraint(line,',');
        std::map<std::string,std::string> val;
        for(std::vector<std::string>::iterator r = row.begin(); r != row.end(); r++){
            std::vector<std::string> col = splitconstraint(*r,'=');
            if(col.size()!=2){
                std::stringstream msg;
                std::cerr << "Malformed hook events file";
                setStatusAndReturn(/*phase*/"parseHookEvents",/*error*/msg.str(), /*success*/"");
                return annotations;
            }
            val[col[0]] = col[1];
        }
        if(row.size()>2){
            std::map<std::string,std::string>::iterator ts_it = val.find(ts);
            if(ts_it != val.end()){
                bool ts_match = false;
                std::string ts_string = ts_it->second;
                long ts_val = atol(ts_string.c_str());
                double ts_now;
                if(using_clocktime){
                    ts_now = double(ts_val)/1000000000.0;
                    ts_match = (ts_val > this->start_clock && ts_val < this->end_clock);
                }
                else{
                    double ts_sec = (double)ts_val/1000.0;
                    time_t ts_time = (long)(ts_sec);
                    struct timeval tv;
                    struct tm * now = localtime( & ts_time );
                    double tm_sec = now->tm_sec +  ts_sec - ts_time;
                    ts_now = now->tm_hour*3600 + now->tm_min*60 + tm_sec;
                    ts_match = (ts_now >= ts_start && ts_now <= ts_end);
                }

                if(ts_match){
                    //std::cout << "when: " << now->tm_hour << "h " << now->tm_min << "m "  << tm_sec << "s " << std::endl;
                    bool cur_event_is_mouse = false;
                    bool cur_event_is_keyboard = false;
                    std::map<std::string,std::string>::iterator keychar_it = val.find("keychar");
                    std::map<std::string,std::string>::iterator keycode_it = val.find("keycode");
                    std::map<std::string,std::string>::iterator rawcode_it = val.find("rawcode");
                    std::map<std::string,std::string>::iterator x_it = val.find("x");
                    std::map<std::string,std::string>::iterator y_it = val.find("y");
                    std::map<std::string,std::string>::iterator button_it = val.find("button");
                    std::map<std::string,std::string>::iterator clicks_it = val.find("clicks");
                    if(keycode_it != val.end() || keychar_it != val.end()){
                        cur_event_is_keyboard = true;
                        if( keychar_it != val.end() ){
                            keychar_str = keychar_it->second;
                            keychar_int = atoi(keychar_str.c_str());
                            keychar_time = ts_now;
                            keychar_canformword = canformword(keychar_str);
                            if(isActionActive["getKeysTyped"]){
                                event(*w_s[annotationName["getKeysTyped"]], (ts_now - ts_start)*fps , this->fps, keychar_str);
                            }
                        }
                        if( keycode_it != val.end()  ){
                            keycode_int = atoi(keycode_it->second.c_str());
                            keycode_time = ts_now;
                            keycode_canformword = canformword(keycode_int);
                            keycode_modifier_string = modifierString(keycode_int);
                            if(!keycode_modifier_string.empty() && isActionActive["getModifierKeysPressed"]){
                                event(*w_s[annotationName["getModifierKeysPressed"]], (ts_now - ts_start)*fps , this->fps, keycode_modifier_string);
                            }
                        }
                        if( rawcode_it != val.end()  ){
                            std::string rawcode_str = rawcode_it->second;
                            size_t rawcodehex_pos = rawcode_str.find("0x");
                            if( rawcodehex_pos == 0  ){
                                rawcode_int = strtoul(rawcode_str.substr(2).c_str(), NULL, 16);
                            }
                            else{
                                rawcode_int = atoi(rawcode_str.c_str());
                            }
                            rawcode_time = ts_now;
                        }
                        if(keycode_time == keychar_time && keychar_canformword/*keycode_canformword*/){
                            if(word.empty()){
                                wordin = (ts_now - ts_start)*fps;
                            }
                            if(keycode_int == VC_BACKSPACE && !word.empty()){
                                word = word.substr(0,word.size()-1);
                            }
                            else{
                                word += keychar_str;
                            }
                            wordout = (ts_now - ts_start)*fps;
                        }
                    }
                    else if(x_it != val.end() || y_it != val.end()){
                        cur_event_is_mouse = true;
                        __x = atof(x_it->second.c_str());
                        __y = atof(x_it->second.c_str());

                    }
                    if(clicks_it != val.end() && button_it != val.end()){
                        cur_event_is_mouse = true;
                        bool clicked = (clicks_it->second == "1");
                        std::string button = "Button: "+ button_it->second;
                        if(clicked && isActionActive["getPointerClicks"]){
                            event(*w_s[annotationName["getPointerClicks"]], (ts_now - ts_start)*fps , this->fps, button);
                        }
                    }

                    if( (prev_event_is_keyboard && !cur_event_is_keyboard) || (cur_event_is_keyboard && !keychar_canformword/*!keycode_canformword*/ ) ){
                        //std::cout << "when: " << now->tm_hour << "h " << now->tm_min << "m "  << tm_sec << "s " ;//<< std::endl;
                        //std::cout << "Prev word'" << word << "'" << std::endl;
                        std::string spacey(word);
                        spacey.erase (std::remove(spacey.begin(), spacey.end(), ' '), spacey.end());

                        if(word.size()>0 && spacey.size()>0){
                            wordout = (ts_now - ts_start)*fps;

                            while(_frame < wordin && _frame < this->video_frames){
                                _x.push_back(0.0);
                                _y.push_back(0.0);
                                _words.push_back(" ");
                                _frame++;
                            }
                            while(_frame < wordout && _frame < this->video_frames){
                                _x.push_back(__x);
                                _y.push_back(__y);
                                _words.push_back(word);
                                _frame++;
                            }

                            if(isActionActive["getWords"]){
                                segment(*w_s[annotationName["getWords"]], wordin, wordout, this->fps, word);
                                //overlay(*w_o[wordshook], wordin, wordout, this->fps, __x/(float)video_w, __y/(float)video_h, 20/(float)video_w, 20/(float)video_h, word);
                                //std::cout << "word '" << word << "' @ " << wordin << " " << wordout << std::endl;
                            }
                        }
                        word.erase();

                        keychar_canformword = false;
                        keycode_canformword = false;
                        keycode_modifier_string = "";
                    }

                    prev_event_is_mouse = cur_event_is_mouse;
                    prev_event_is_keyboard = cur_event_is_keyboard;
                }
            }

        }
        r++;
    }

    for(std::vector<std::string>::iterator name = names.begin(); name != names.end();name++ ){
        std::string label = *name;
        //overlayfooter(*w_o[*name],label,wordout);

        std::ofstream segmentfile;
        std::string segmentfilepath;

        if( (isActionActive["getPointerClicks"] && annotationName["getPointerClicks"] == *name)
                || (isActionActive["getKeysTyped"] && annotationName["getKeysTyped"] == *name)
                || (isActionActive["getModifierKeysPressed"] && annotationName["getModifierKeysPressed"] == *name)
                ){
            segmentfilepath = datapath + videostem + std::string("-") + label + std::string("-events.json");
            eventfooter(*w_s[*name],"input_events",label,this->video_frames,this->fps);
        }
        else if( (isActionActive["getWords"] && annotationName["getWords"] == *name) ){
            segmentfilepath = datapath + videostem + std::string("-") + label + std::string("-segments.json");
            segmentfooter(*w_s[*name],label,"input_events",this->video_frames,this->fps);
        }

        segmentfile.open(segmentfilepath.c_str());
        if(segmentfile.is_open()){
            segmentfile << s_s[*name].GetString();
            segmentfile.close();
        }
        annotations.push_back(s_s[*name].GetString());
        delete(w_o[*name]);
        delete(w_s[*name]);
    }
    s_o.clear();
    s_s.clear();
    w_o.clear();
    w_s.clear();

    hook_txt.close();

    stop = getTickCount();
    time = (double)(stop-start)/frequency;
    std::cout << "Time taken to parse hook events and save jsons: " << time << std::endl;

    return annotations;
}

std::string InspectorWidgetProcessor::getTemplateAnnotation(std::string name){
    std::string _annotation("");
    std::map<std::string, std::vector<float> >::iterator _template_val = template_vals.find(name);
    if(_template_val!=template_vals.end()){
        int stop;
        double time;
        int start = getTickCount();
        double frequency = getTickFrequency();

        StringBuffer s_s;

        PrettyWriter<StringBuffer>* w_s = new PrettyWriter<StringBuffer>(s_s);

        header(*w_s);

        float _val = 0;
        float __val = 0;
        int _in = 0;

        int f=0;
        for(;f<template_vals[name].size();f++) {
            _val = (template_vals[name][f]>_threshold)?1:0;
            if ( _val == 0 && __val == 1){
                segment(*w_s, _in, f, this->fps, name);
            }
            if ( _val == 1 && __val == 0){
                _in = f;
            }
            __val = _val;
        }
        if(_val == 1){
            segment(*w_s, _in, f, this->fps, name);
        }
        segmentfooter(*w_s, name,"computer_vision", f, this->fps);

        _annotation = s_s.GetString();

        stop = getTickCount();
        time = (double)(stop-start)/frequency;
    }
    return _annotation;
}

float InspectorWidgetProcessor::getElapsedSeconds(float timestamp){

    struct tm _start_tm;
    _start_tm.tm_year = this->start_d.y - 1900;
    _start_tm.tm_mon = this->start_d.m - 1;
    _start_tm.tm_mday = this->start_d.d;
    _start_tm.tm_hour = this->start_t.h;
    _start_tm.tm_min = this->start_t.m;
    _start_tm.tm_sec = this->start_t.s;
    std::cout << "Begin " << this->start_d.y << " " << this->start_d.m << " " << this->start_d.d << " " << this->start_t.h << " " << this->start_t.m << " " << this->start_t.s << std::endl;

    time_t _start_t = mktime(&_start_tm);

    time_t _end_t = (long)(timestamp);
    struct tm * _end_tm = localtime( & _end_t );
    std::cout << "End " << _end_tm->tm_year + 1900 << " " << _end_tm->tm_mon + 1 << " " << _end_tm->tm_mday << " " << _end_tm->tm_hour << " " << _end_tm->tm_min << " " << _end_tm->tm_sec << std::endl;

    double _diff = difftime(_start_t,_end_t);

    std::cout << "Diff " << _diff << " " << this->start_t.s - (int) (this->start_t.s) << " " <<  timestamp -  (long)(timestamp) << " " << timestamp << std::endl;

    return _diff;
}

std::vector<std::string> InspectorWidgetProcessor::getAnnotations(std::vector<std::string> names){
    std::vector<std::string> annotations;

    std::vector<std::string> _templates;
    std::vector<std::string> _ax_annotations;
    std::vector<std::string> _inputhooks;

    for(std::vector<std::string>::iterator name = names.begin();name!=names.end();name++){
        std::vector<std::string>::iterator _template = std::find(template_list.begin(),template_list.end(),*name);
        std::vector<std::string>::iterator _ax_annotation = std::find(ax_annotations.begin(),ax_annotations.end(),*name);
        std::vector<std::string>::iterator _inputhook = std::find(inputhooks.begin(),inputhooks.end(),*name);
        if(_template!=template_list.end()){
            _templates.push_back(*name);
        }
        else if(_ax_annotation!=ax_annotations.end()){
            _ax_annotations.push_back(*name);
        }
        else if(_inputhook!=inputhooks.end()){
            _inputhooks.push_back(*name);
        }
        else{
            std::stringstream msg;
            msg << "Annotation " << *name << " is not defined";
            setStatusAndReturn(/*phase*/"getAnnotations",/*error*/msg.str(), /*success*/"");
            return annotations;
        }
    }

    /// Empty names vector means return all available annotations
    if(names.empty()){
        for(std::vector<std::string>::iterator t = template_list.begin();t!=template_list.end();t++){
            _templates.push_back(*t);
        }
        for(std::vector<std::string>::iterator a = ax_annotations.begin();a!=ax_annotations.end();a++){
            _ax_annotations.push_back(*a);
        }
        for(std::vector<std::string>::iterator h = inputhooks.begin();h!=inputhooks.end();h++){
            _inputhooks.push_back(*h);
        }
    }

    for(std::vector<std::string>::iterator _template = _templates.begin();_template!=_templates.end();_template++){
        std::string _annotation = this->getTemplateAnnotation(*_template);
        if(!_annotation.empty()) annotations.push_back( _annotation );
    }
    for(std::vector<std::string>::iterator _ax_annotation = _ax_annotations.begin();_ax_annotation!=_ax_annotations.end();_ax_annotation++){
        std::vector<std::string> _annotations = this->getAccessibilityAnnotations(_ax_annotations);
        for(std::vector<std::string>::iterator _annotation = _annotations.begin(); _annotation != _annotations.end(); _annotation++){
            if(!_annotation->empty())  annotations.push_back(*_annotation);
        }
    }
    if(!_inputhooks.empty()){
        this->hook_path = datapath + videostem + ".txt";
        std::vector<std::string> _annotations = this->parseHookEvents(this->hook_path,_inputhooks,true);
        for(std::vector<std::string>::iterator _annotation = _annotations.begin(); _annotation != _annotations.end(); _annotation++){
            if(!_annotation->empty())  annotations.push_back(*_annotation);
        }
    }

    return annotations;
}

std::vector<std::string> InspectorWidgetProcessor::getAccessibilityAnnotations(std::vector<std::string> names){
    std::vector<std::string> annotations;

    /// Check if names are listed in registered input hook definitions in the pipeline
    for(std::vector<std::string>::iterator name = names.begin(); name != names.end();name++ ){
        if( std::find(ax_annotations.begin(), ax_annotations.end(), *name) == ax_annotations.end()){
            std::stringstream msg;
            std::cerr <<  *name << "is not part of registered accessibility definitions in the pipeline, aborting";
            setStatusAndReturn(/*phase*/"getAccessibilityAnnotations",/*error*/msg.str(), /*success*/"");
            return annotations;
        }
    }

    pugi::xml_document doc;

    std::string axFile = datapath + videostem + ".xml";

    std::ifstream iss( axFile );

    //iss.exceptions(std::ios::eofbit | std::ios::badbit | std::ios::failbit);

    // Windows has newline translation for text-mode files, so reading from this stream reaches eof and sets fail|eof bits.
    // This test does not cause stream to throw an exception on Linux - I have no idea how to get read() to fail except
    // newline translation.
    pugi::xml_parse_result result;

    try
    {
        result = doc.load(iss);
        //std::cout << "Stream read: " << iss.good() << std::endl; // if the exception was not thrown, stream reading should succeed without errors
    }
    catch (const std::ios_base::failure&)
    {
        //!doc.first_child();
        std::stringstream msg;
        msg << "Could not parse accessibility XML file "<< axFile << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return annotations;
    }

    if(result.status != pugi::status_ok){
        std::stringstream msg;
        msg << "Could not properly load XML file "<< axFile << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return annotations;
    }

    pugi::xml_node fc = doc.first_child();
    if(fc.empty()){
        std::stringstream msg;
        msg << "XML file "<< axFile << " is empty, aborting." << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return annotations;
    }

    pugi::xml_node root = doc.child("root");
    if(root.empty()){
        std::stringstream msg;
        msg << "XML document doesn't contain a root element" << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return annotations;
    }

    int stop;
    double time;
    int start = getTickCount();
    double frequency = getTickFrequency();

    std::map<std::string,StringBuffer> s_o,s_s;
    std::map<std::string,PrettyWriter<StringBuffer>* > w_o,w_s;

    bool getFocusApplication = false;
    std::string getFocusApplicationAnnotation("");
    bool getFocusWindow = false;
    std::string getFocusWindowAnnotation("");
    bool getPointedWidget = false;
    std::string getPointedWidgetAnnotation("");
    bool getWorkspaceSnapshot = false;
    std::string getWorkspaceSnapshotAnnotation("");

    for(std::vector<std::string>::iterator name = names.begin(); name != names.end();name++ ){
        s_o[*name] = StringBuffer();
        s_s[*name] = StringBuffer();
        PrettyWriter<StringBuffer>* _o = new PrettyWriter<StringBuffer>(s_o[*name]);
        PrettyWriter<StringBuffer>* _s = new PrettyWriter<StringBuffer>(s_s[*name]);
        w_o[*name] = _o;
        w_s[*name] = _s;
        header(*_o);
        header(*_s);

        if(ax_action[*name] == "getFocusApplication"){
            getFocusApplication = true;
            getFocusApplicationAnnotation = *name;
        }
        else if(ax_action[*name] == "getFocusWindow"){
            getFocusWindow = true;
            getFocusWindowAnnotation = *name;
        }
        else if(ax_action[*name] == "getPointedWidget"){
            getPointedWidget = true;
            getPointedWidgetAnnotation = *name;
        }
        else if(ax_action[*name] == "getWorkspaceSnapshot"){
            getWorkspaceSnapshot = true;
            getWorkspaceSnapshotAnnotation = *name;
        }
    }

    if(getFocusApplication){

        std::string _name("");

        for (pugi::xml_node n: root.children("appchange"))
        {
            uint64_t _clock = n.attribute("clock").as_llong();
            //std::cout << "appchange " << n.attribute("name").as_string();
            if(_clock > this->end_clock){
                break;
            }
            if(_clock > this->start_clock && _clock < this->end_clock){
                _name = n.attribute("name").as_string();
                double _event_t = double(_clock - this->start_clock )/1000000000.0;
                event(*w_s[getFocusApplicationAnnotation], _event_t*this->fps,this->fps, _name );
            }
        }
        eventfooter(*w_s[getFocusApplicationAnnotation], getFocusApplicationAnnotation,"accessibility", this->video_frames, this->fps);
    }
    if(getFocusWindow){
        uint64_t _last_clock = 0;
        std::string _title("");
        for (pugi::xml_node n: root.children("windowEvent"))
        {
            /*std::cout << n.attribute("time").as_float() << " ";*/
            uint64_t _clock = n.attribute("clock").as_llong();
            if(_clock > this->end_clock){
                break;
            }
            pugi::xml_node t = n.child("target");
            if(!t.empty()){
                //std::cout << "appchange " << n.attribute("name").as_string();
                if(_clock > this->start_clock && _clock < this->end_clock && _last_clock != _clock){
                    _title = t.attribute("title").as_string();
                    double _event_t = double(_clock - this->start_clock )/1000000000.0;
                    //std::cout << "focus '" << t.attribute("title").as_string() << "':'" << t.attribute("app").as_string() << "' ";
                    event(*w_s[getFocusWindowAnnotation], _event_t*this->fps, this->fps, _title );
                }
                _last_clock = _clock;
            }
        }
        eventfooter(*w_s[getFocusWindowAnnotation], getFocusWindowAnnotation, "accessibility", this->video_frames, this->fps);
    }
    if(getPointedWidget){
        std::string _name("");
        for (pugi::xml_node n: root.children("mouse"))
        {
            uint64_t _clock = n.attribute("clock").as_llong();
            //std::cout << "appchange " << n.attribute("name").as_string();
            if(_clock > this->end_clock){
                break;
            }
            if(_clock > this->start_clock && _clock < this->end_clock){
                double _event_t = double(_clock - this->start_clock )/1000000000.0;
                //std::cout << "under mouse";
                try{
                    pugi::xpath_node tpath = n.select_single_node(".//*[@selected='YES']");

                    if (tpath)
                    {
                        pugi::xml_node t = tpath.node();
                        //std::cout << " -> " << t.name() << std::endl;
                        _name = std::string(t.name()) + ": " + t.attribute("AXTitle").as_string();
                    }
                }
                catch(...){
                    std::cout << "Bad xpath" << std::endl;
                }
                event(*w_s[getPointedWidgetAnnotation], _event_t*this->fps, this->fps, _name );
            }
        }
        eventfooter(*w_s[getPointedWidgetAnnotation], getPointedWidgetAnnotation,"accessibility",this->video_frames, this->fps);
    }
    if(getWorkspaceSnapshot){
        for (pugi::xml_node n: root.children("application"))
        {
            uint64_t _clock = n.attribute("clock").as_llong();
            //std::cout << "application " << n.attribute("name").as_string();
            if(_clock > this->end_clock){
                break;
            }
            if(_clock > this->start_clock && _clock < this->end_clock){
                double _event_t = double(_clock - this->start_clock )/1000000000.0;
                //std::cout << "application";
                event(*w_s[getWorkspaceSnapshotAnnotation], _event_t*this->fps, this->fps, getWorkspaceSnapshotAnnotation );
            }
        }
        eventfooter(*w_s[getWorkspaceSnapshotAnnotation], getWorkspaceSnapshotAnnotation,"accessibility",this->video_frames, this->fps);
    }

    for(std::vector<std::string>::iterator name = names.begin(); name != names.end();name++ ){
        std::string label = *name;
        std::ofstream segmentfile;
        std::string segmentfilepath = datapath + videostem + std::string("-") + label + std::string("-events.json");
        segmentfile.open(segmentfilepath.c_str());
        if(segmentfile.is_open()){
            segmentfile << s_s[*name].GetString();
            segmentfile.close();
        }
        annotations.push_back(s_s[*name].GetString());
    }
    for(std::map<std::string,PrettyWriter<StringBuffer>* >::iterator _s = w_s.begin(); _s != w_s.end(); _s++){
        if(_s->second){
            delete _s->second;
        }
    }
    for(std::map<std::string,PrettyWriter<StringBuffer>* >::iterator _o = w_o.begin(); _o != w_o.end(); _o++){
        if(_o->second){
            delete _o->second;
        }
    }
    s_o.clear();
    s_s.clear();
    w_o.clear();
    w_s.clear();

    stop = getTickCount();
    time = (double)(stop-start)/frequency;
    std::cout << "Time taken to get accessibility annotations and save jsons: " << time << std::endl;

    return annotations;
}

InspectorWidgetAccessibilityHoverInfo InspectorWidgetProcessor::getAccessibilityHover(float _time, float _x, float _y){
    InspectorWidgetAccessibilityHoverInfo info;
    std::vector<float> rect(4,0.0);
    std::string axTreeChildren,axTreeParents;
    std::string axTreeIndent(" ");
    const unsigned int axTreeFormat = pugi::format_indent | pugi::format_no_declaration | pugi::format_save_file_text;
    pugi::xml_encoding axTreeEncoding = pugi::encoding_auto;
    int axTreeDepth = 0;

    uint64_t time = this->start_clock + _time*1000000000;

    if(ax_hover_x ==_x && ax_hover_y ==_y && ax_hover_time ==_time && !ax_hover_tree_children.empty() && !ax_hover_tree_parents.empty()){
        info.xml_tree_children = ax_hover_tree_children;
        info.xml_tree_parents = ax_hover_tree_parents;
        info.rect = ax_hover_rect;
        return info;
    }

    pugi::xml_node ax_hover_root;
    bool ax_hover_root_parsed;
    pugi::xml_node ax_hover_closest_node;
    pugi::xml_node ax_hover_closest_window_node;

    /// Parse the XML tree (if not yet done)
    //if(!ax_hover_root_parsed){

    pugi::xml_document doc;
    std::string axFile = datapath + videostem + ".xml";
    std::ifstream iss( axFile );
    //iss.exceptions(std::ios::eofbit | std::ios::badbit | std::ios::failbit);

    // Windows has newline translation for text-mode files, so reading from this stream reaches eof and sets fail|eof bits.
    // This test does not cause stream to throw an exception on Linux - I have no idea how to get read() to fail except
    // newline translation.
    pugi::xml_parse_result result;

    try
    {
        result = doc.load(iss);
        //std::cout << "Stream read: " << iss.good() << std::endl; // if the exception was not thrown, stream reading should succeed without errors
    }
    catch (const std::ios_base::failure&)
    {
        //!doc.first_child();
        std::stringstream msg;
        msg << "Could not parse accessibility XML file "<< axFile << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return info;
    }

    if(result.status != pugi::status_ok){
        std::stringstream msg;
        msg << "Could not properly load XML file "<< axFile << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return info;
    }

    pugi::xml_node fc = doc.first_child();
    if(fc.empty()){
        std::stringstream msg;
        msg << "XML file "<< axFile << " is empty, aborting." << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return info;
    }

    ax_hover_root = doc.child("root");
    if(ax_hover_root.empty()){
        std::stringstream msg;
        msg << "XML document doesn't contain a root element" << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return info;
    }
    ax_hover_root_parsed = true;
    //}

    //std::cout << "getAccessibilityHover: time: " << _time << " x: "<< _x << " y: "<< _y<<std::endl;

    /// Find the closest node before time
    pugi::xml_node closest_node;// = ax_hover_closest_node;
    pugi::xml_node closest_window_node;// = ax_hover_closest_window_node;
    uint64_t closest_node_clock = 0;
    uint64_t closest_window_node_clock = 0;
    uint64_t closest_application_node_clock = 0;
    //if( ax_hover_time != _time || closest_node.empty() || closest_window_node.empty()){
    pugi::xml_node _prev_node;
    uint64_t _prev_clock = this->start_clock;
    //for (pugi::xml_node n = ax_hover_root.last_child(); !n.empty(); n = n.previous_sibling())
    for (pugi::xml_node n: ax_hover_root.children())
    {
        uint64_t _clock = n.attribute("clock").as_llong();
        //std::cout  << "prev=" << _prev_clock   << " time=" << time  << " next=" << _clock << " start="<< this->start_clock << " end="<< this->end_clock << " name=" << n.name() << std::endl;
        if(_clock > this->start_clock && _clock < this->end_clock && _prev_clock < time && time < _clock){
            //std::cout << "here" << std::endl;
            closest_node = n;
            closest_node_clock = _clock;
            break;
        }
        _prev_clock = _clock;
        _prev_node = n;
    }

    if(closest_node.empty()){
        std::stringstream msg;
        msg << "Could not match the closest XML element before the desired time" << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return info;
    }

    //std::cout << "- closest node at " << closest_node_clock << std::endl;

    /// Find the closest preceeding windowEvent node before the closest node before time
    for (pugi::xml_node n = closest_node; !n.empty(); n = n.previous_sibling()){
        //std::cout << "name=" << n.name() << std::endl;
        if(std::string(n.name()) == "windowEvent" && !n.child("allWindows").empty()){
            closest_window_node = n;
            closest_window_node_clock = n.attribute("clock").as_llong();
            break;
        }
    }

    if(closest_window_node.empty()){
        std::stringstream msg;
        msg << "Could not match the closest windowEvent XML element before the desired time" << std::endl;
        this->setStatusAndReturn("accessibility", msg.str(), "");
        return info;
    }
    ax_hover_closest_node = closest_node;
    ax_hover_closest_window_node = closest_window_node;
    //}
    ax_hover_time = _time;

    //std::cout << "-- closest preceeding windowEvent at " << closest_window_node_clock << std::endl;

    /// List all windows that contain x and y
    std::string windowTitle;
    pugi::xml_node allWindows;

    if( !(ax_hover_x==_x && ax_hover_y==_y) && !closest_node.empty() /*&& !closest_window_node.empty()*/ ){
        allWindows = closest_window_node.child("allWindows");
        //if(!allWindows.empty()){
        for (pugi::xml_node n: closest_window_node.child("allWindows").children("window")){
            float wx = n.attribute("x").as_float();
            float wy = n.attribute("y").as_float();
            float ww = n.attribute("w").as_float();
            float wh = n.attribute("h").as_float();
            float wl = wx/float(this->video_w);
            float wr = (wx+ww)/float(this->video_w);
            float wt = wy/float(this->video_h);
            float wb = (wy+wh)/float(this->video_h);

            //        std::cout << "@ " << closest_window_node.attribute("clock").as_llong();
            //        std::cout << " window " << n.attribute("app").as_string() << ":" << n.attribute("title").as_string() ;
            //        std::cout << " wl=" << wl << " wr=" << wr << " wt=" << wt << " wb=" << wb << " ";
            //        std::cout << " x=" << x << " y=" << y;

            if( wl <= _x && _x <= wr && wt <= _y && _y <= wb){
                /// For each window that contains x and y, find the closest preceeding application node, parse to find the appropriate widget
                //            std::cout << " matches ";

                /// Use window dimensions
                rect[0] = wl;
                rect[1] = wt;
                rect[2] = wr-wl;
                rect[3] = wb-wt;
                std::stringstream axTreeChildrenStream;
                n.print(axTreeChildrenStream, PUGIXML_TEXT(axTreeIndent.c_str()), axTreeFormat, axTreeEncoding, axTreeDepth);
                axTreeChildren = axTreeChildrenStream.str();
                ax_hover_rect = rect;

                windowTitle = n.attribute("title").as_string();

                /// Find the closest preceeding application node
                for (pugi::xml_node t = closest_node; !t.empty(); t = t.previous_sibling()){
                    if(std::string(t.name()) == "application"){
                        std::string query = std::string("./AXApplication/AXWindow[@AXTitle='") + n.attribute("title").as_string() + "']";
                        pugi::xpath_node tpath = t.select_single_node(query.c_str());

                        if (tpath)
                        {
                            pugi::xml_node w = tpath.node();
                            //                        std::cout << std::endl;
                            //                        std::cout << " @ " << t.attribute("clock").as_llong();
                            //                        std::cout << " name " << w.name();// << std::endl;
                            //                        std::cout << " AXTitle " << w.attribute("AXTitle").as_string() << std::endl;


                            closest_application_node_clock = t.attribute("clock").as_llong();
                            //std::cout << "--- closest preceeding application at " << closest_application_node_clock << std::endl;

                            /// Parse all window children to find a more precise target, depth first
                            //std::map<float, std::vector<float> > _rects;
                            //std::map<float, std::string > _axTreeChildrens;
                            std::map<float, InspectorWidgetAccessibilityHoverInfo> _infos;

                            std::string _tab(" ");
                            pugi::xml_node _w = w.first_child();
                            pugi::xml_node _nfc;
                            while( !_w.empty() ){
                                bool matching_widget = false;
                                pugi::xml_node c = _w;
                                /// Make sure to parse all first-order children of the window
                                if(c.parent() == w){
                                    _nfc = c.next_sibling();
                                    //                                std::cout << " (nfc=" << _nfc.name() << ") ";
                                }
                                /// Find location and dimensions of the widget
                                float _wx = 0.0;
                                float _wy = 0.0;
                                float _ww = 0.0;
                                float _wh = 0.0;
                                /// First try with AXFrame attribute
                                std::string _wf = c.attribute("AXFrame").as_string();
                                if(!_wf.empty()){
                                    std::vector<std::string> _wps = splitconstraint(_wf,' ');
                                    //                            std::cout << _tab;
                                    //                            std::cout << " child " << c.name();
                                    //                            std::cout << " AXTitle " << c.attribute("AXFrame").as_string();
                                    //                            std::cout << " AXFrame " << _wf;
                                    for(std::vector<std::string>::iterator _wp = _wps.begin(); _wp!=_wps.end();_wp++){
                                        if(!_wp->empty()){
                                            std::string _wn = _wp->substr(0,1);
                                            std::string _wv = _wp->substr(2,_wp->length()-1);
                                            if(_wn == "x"){
                                                _wx = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "y"){
                                                _wy = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "w"){
                                                _ww = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "h"){
                                                _wh = atoi(_wv.c_str());
                                            }
                                        }
                                    }
                                }
                                else{
                                    //std::cout << "---- AXFrame not found " << std::endl;
                                    /// Otherwise try with AXPosition and AXSize attributes
                                    std::string _wp = c.attribute("AXPosition").as_string();
                                    std::string _ws = c.attribute("AXSize").as_string();
                                    /*if(_wp.empty()){
                                        std::cout << "---- AXPosition not found " << std::endl;
                                    }
                                    if(_ws.empty()){
                                        std::cout << "---- AXSize not found " << std::endl;
                                    }*/
                                    std::vector<std::string> _wpcs = splitconstraint(_wp,' ');
                                    std::vector<std::string> _wscs = splitconstraint(_ws,' ');
                                    for(std::vector<std::string>::iterator _wpc = _wpcs.begin(); _wpc!=_wpcs.end();_wpc++){
                                        if(!_wpc->empty()){
                                            std::string _wn = _wpc->substr(0,1);
                                            std::string _wv = _wpc->substr(2,_wpc->length()-1);
                                            if(_wn == "x"){
                                                _wx = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "y"){
                                                _wy = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "w"){
                                                _ww = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "h"){
                                                _wh = atoi(_wv.c_str());
                                            }
                                        }
                                    }
                                    for(std::vector<std::string>::iterator _wps = _wscs.begin(); _wps!=_wscs.end();_wps++){
                                        if(!_wps->empty()){
                                            std::string _wn = _wps->substr(0,1);
                                            std::string _wv = _wps->substr(2,_wps->length()-1);
                                            if(_wn == "x"){
                                                _wx = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "y"){
                                                _wy = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "w"){
                                                _ww = atoi(_wv.c_str());
                                            }
                                            else if(_wn == "h"){
                                                _wh = atoi(_wv.c_str());
                                            }
                                        }
                                    }
                                }

                                float _wl = _wx/float(this->video_w);
                                float _wr = (_wx+_ww)/float(this->video_w);
                                float _wt = _wy/float(this->video_h);
                                float _wb = (_wy+_wh)/float(this->video_h);
                                //std::cout << "---- "<< c.   name() << " title=" << c.attribute("AXTitle").as_string() << " wl=" << _wl << " wr=" << _wr << " wt=" << _wt << " wb=" << _wb << " ";
                                //                            std::cout << " wl=" << _wl << " wr=" << _wr << " wt=" << _wt << " wb=" << _wb << " ";
                                //                            std::cout << " x=" << x << " y=" << y;

                                if( _wl < _x && _x < _wr && _wt < _y && _y < _wb){
                                    //std::cout << " MATCHES ";
                                    std::vector<float> _rect(4,0.0);
                                    _rect[0] = _wl;
                                    _rect[1] = _wt;
                                    _rect[2] = _wr-_wl;
                                    _rect[3] = _wb-_wt;

                                    /// Map all rect candidates by area
                                    float area = (_wr-_wl)*(_wb-_wt);
                                    InspectorWidgetAccessibilityHoverInfo _info;
                                    _info.rect = _rect;
                                    _info.xml_node = c;
                                    _infos[area] = _info;
                                    //std::cout << " area=" << area;
                                     //std::cout << std::endl;
                                    _w = _w.first_child();
                                    _tab += " ";
                                    matching_widget = true;
                                }
                                else{
                                    _w = _w.next_sibling();
                                }

                                if(_w.empty()){
                                    _w = _nfc;
                                    _tab = " ";
                                }

                                //std::cout << std::endl;
                            }
                            /// Choose the rect candidate with minimal area (if any)
                            //std::cout << "--- candidates:" << _infos.size() << std::endl;
                            pugi::xml_node c;
                            if(_infos.size()>0){
                                ax_hover_rect = _infos.begin()->second.rect;
                                c = _infos.begin()->second.xml_node;
                            }
                            else{
                                c = w;
                                std::cerr << "No matching widget for window titled " << n.attribute("title").as_string() << std::endl;
                            }

                            //axTreeChildren = _infos.begin()->second.xml_tree_children;
                            //axTreeParents = _infos.begin()->second.xml_tree_parents;

                            std::stringstream _axTreeChildrenStream;
                            c.print(_axTreeChildrenStream, PUGIXML_TEXT(axTreeIndent.c_str()), axTreeFormat, axTreeEncoding, axTreeDepth);
                            axTreeChildren = _axTreeChildrenStream.str();

                            pugi::xml_node _p,_c;
                            _c = c;
                            //std::string indent("");
                            pugi::xml_document  _d;
                            while (!_c.parent().empty() && std::string(_c.first_child().name()) != "AXApplication"){
                                pugi::xml_node __c = _d.append_child(_c.name());
                                //std::cout << indent << _c.name() << std::endl;
                                __c.set_value(_c.value());
                                for (pugi::xml_attribute _a: _c.attributes()){
                                    //std::cout << indent << "-" << _a.name() << std::endl;
                                    pugi::xml_attribute __a = __c.append_attribute(_a.name());
                                    __a.set_value(_a.value());
                                }
                                __c.append_copy(_p);
                                _p = __c;
                                _c = _c.parent();
                                //indent += " ";
                            }
                            std::stringstream _axTreeParentsStream;
                            _p.print(_axTreeParentsStream, PUGIXML_TEXT(axTreeIndent.c_str()), axTreeFormat, axTreeEncoding, axTreeDepth);
                            axTreeParents = _axTreeParentsStream.str();

                            //                        std::cout << std::endl;
                            break;
                        }
                    }
                }
                break;
            }
            //        std::cout << std::endl;
        }
        //}
    }
    if(axTreeChildren.empty()){
        ax_hover_tree_children = "";
        ax_hover_rect = std::vector<float>(4,0.0);
        axTreeChildren = "empty";
    }
    else if(axTreeChildren == ax_hover_tree_children){
        axTreeChildren = "identical";
    }
    else{
        ax_hover_tree_children = axTreeChildren;
    }
    if(axTreeParents.empty()){
        ax_hover_tree_parents = "";
        //ax_hover_rect = std::vector<float>(4,0.0);
        axTreeParents = "empty";
    }
    else if(axTreeParents == ax_hover_tree_parents){
        axTreeParents = "identical";
    }
    else{
        ax_hover_tree_parents = axTreeParents;
    }
    ax_hover_x =_x;
    ax_hover_y =_y;
    info.xml_tree_children = axTreeChildren;
    info.xml_tree_parents = axTreeParents;
    //std::cout << "Parents " << axTreeParents << std::endl;
    std::cout << "- rect "<< ax_hover_rect[0] << " " << ax_hover_rect[1] << " " << ax_hover_rect[2] << " " << ax_hover_rect[3] << " " <<std::endl;
    info.rect = ax_hover_rect;
    return info;
}

bool InspectorWidgetProcessor::parseComputerVisionEvents(PCP::CsvConfig* cv_csv){

    if(!cv_csv){
        return 0;
    }

    int stop;
    double time;
    int start = getTickCount();
    double frequency = getTickFrequency();

    // parse header line
    std::vector<std::string> headers = cv_csv->get_headers();

    int annotations = (headers.size()-1)/3.0;
    std::cout << "Annotations: " << annotations << std::endl;

    std::vector<std::string> label(annotations);
    std::vector<int> in(annotations,0);
    std::vector<float> val(annotations,0.0);
    std::vector<std::string> num(annotations);
    std::vector<std::string> txt(annotations);
    std::vector<std::string> ts(annotations);
    std::vector<int> rx(annotations,0);
    std::vector<int> ry(annotations,0);

    std::vector<StringBuffer> s_o(annotations),s_s(annotations);

    std::vector < PrettyWriter<StringBuffer>* > w_o,w_s;

    std::vector<std::string> label_type(annotations);

    for(size_t i = 0; i < annotations; ++i){
        std::string _label = headers[3*i+3];
        //std::replace( _label.begin(), _label.end(), char('\"'), char(' '));

        _label.erase (std::remove(_label.begin(), _label.end(), '\"'), _label.end());

        std::size_t val_pos = _label.find("_val");
        if(val_pos!=std::string::npos){
            _label = _label.substr (0,val_pos);
            label_type[i] = "val";
        }
        std::size_t txt_pos = _label.find("_txt");
        if(txt_pos!=std::string::npos){
            _label = _label.substr (0,txt_pos);
            label_type[i] = "txt";
        }
        std::size_t num_pos = _label.find("_num");
        if(num_pos!=std::string::npos){
            _label = _label.substr (0,num_pos);
            label_type[i] = "num";
        }
        std::size_t time_pos = _label.find("_time");
        if(time_pos!=std::string::npos){
            _label = _label.substr (0,time_pos);
            label_type[i] = "time";
        }

        label[i] = _label;

        if(label_type[i] == "val"){

            std::string _templatefile = datapath + _label + ".png";

            cv::Mat _template = cv::imread( _templatefile,1 );
            if(_template.empty()){
                std::cerr << "Template " << _templatefile << " could not be opened, aborting..." << std::endl;
                return 0;
            }
            rx[i] = _template.cols;
            ry[i] = _template.rows;
            _template.release();

        }

        PrettyWriter<StringBuffer>* _o = new PrettyWriter<StringBuffer>(s_o[i]);
        PrettyWriter<StringBuffer>* _s = new PrettyWriter<StringBuffer>(s_s[i]);

        w_o.push_back( _o );
        w_s.push_back( _s );

        header(*_o);
        header(*_s);

        std::cout << label[i] << std::endl;
    }

    // print headers
    std::cout << "Headers:" << std::endl;
    for (size_t i = 0; i < headers.size(); ++i)
        std::cout << headers[i] << "\t" /*<< (i-1)%3 << "\n"*/;
    std::cout << std::endl;

    // instantiate parser
    PCP::PartialCsvParser parser(*cv_csv);  // parses whole body of CSV without range options.

    int r = 0;
    int f = 0;
    float _val = 0;
    std::string _num;
    std::string _txt;
    std::string _time;
    std::vector<float> _x(annotations,-1),_y(annotations,-1);
    int _in = 0;
    std::vector< std::vector<int> > _h(annotations),_m(annotations),_s(annotations);

    // parse & print body lines
    std::vector<std::string> row;
    while (!(row = parser.get_row()).empty()) {

        /*            std::cout << "Got a row: ";
                  for (size_t i = 0; i < row.size(); ++i){
                      std::cout << row[i] << "\t";
                  }
                  //std::cout << std::endl;
      */
        if( row.size() == headers.size()){
            _in = atoi(row[0].c_str());
            f = _in;
            //std::cout << " frame=" << _in << "\t";
            for(size_t i = 0; i < annotations; ++i){

                if(label_type[i] == "val"){

                    _val = atof(row[3*i+3].c_str());
                    _val = (_val>_threshold)?1:0;
                    //std::cout << " _val='" << _val << "'' ";

                    log_val[label[i]].push_back(_val);

                    if ( val[i] != _val){

                        std::cout << "row " << r << " label=" << label[i] << " samples="  << _in - in[i] << " val='" << val[i] << "'" << std::endl;;

                        if(val[i] > _threshold){
                            segment(*(w_s[i]), in[i], _in, this->fps, label[i]);
                            overlay(*(w_o[i]), in[i], _in, this->fps,
                                    (float)_x[i]/(float)video_w+0.5*(float)rx[i]/(float)video_w,
                                    (float)_y[i]/(float)video_h+0.5*(float)ry[i]/(float)video_h,
                                    0.5*(float)rx[i]/(float)video_w,
                                    0.5*(float)ry[i]/(float)video_h,
                                    label[i]
                                    );
                        }

                        in[i] = _in;
                        val[i] = _val;
                    }
                }
                else if (label_type[i] == "num"){
                    _num = row[3*i+3];
                    log_txt[label[i]].push_back(_num);
                    if( _num!=num[i]){
                        if(num[i]!=" " && !num[i].empty()){
                            segment(*(w_s[i]), in[i], _in, this->fps, num[i]);
                            overlay(*(w_o[i]), in[i], _in, this->fps,
                                    (float)_x[i]/(float)video_w,
                                    (float)_y[i]/(float)video_h,
                                    0.05,
                                    0.05,
                                    num[i]
                                    );
                        }
                        in[i] = _in;
                        num[i] = _num;
                    }

                }
                else if (label_type[i] == "txt"){
                    _txt = row[3*i+3];
                    log_txt[label[i]].push_back(_txt);
                    if(_txt != txt[i]){
                        if(txt[i]!=" " && !txt[i].empty() ){
                            segment(*(w_s[i]), in[i], _in, this->fps, txt[i]);
                            overlay(*(w_o[i]), in[i], _in, this->fps,
                                    (float)_x[i]/(float)video_w,
                                    (float)_y[i]/(float)video_h,
                                    0.05,
                                    0.05,
                                    txt[i]
                                    );
                        }
                        in[i] = _in;
                        txt[i] = _txt;
                    }
                }
                else if (label_type[i] == "time"){
                    _time = row[3*i+3];
                    log_txt[label[i]].push_back(_time);
                    std::string::const_iterator _t = _time.begin();
                    bool _isdigit = false;
                    std::string _d;
                    std::vector<int> _n;
                    for(;_t != _time.end();_t++){
                        if(!_isdigit){
                            _d.clear();
                        }
                        bool __isdigit = std::isdigit(*_t);
                        if(__isdigit){
                            _d+= *_t;
                        }
                        else if(!_d.empty()){
                            _n.push_back( atoi(_d.c_str()) );
                        }
                        _isdigit = __isdigit;
                    }
                    if(!_d.empty() && _isdigit){
                        _n.push_back( atoi(_d.c_str()) );
                    }
                    //std::cout << "Found " << _n.size() << " digit(s)" << std::endl;
                    int __h(-1),__m(-1);
                    if(_n.size() == 2){
                        __h = _n[0];
                        __m = _n[1];
                    }
                    //std::cout << "Time " << __h << ":" << __m << std::endl;
                    _h[i].push_back( __h );
                    _m[i].push_back( __m );
                }

                _x[i] = atof(row[3*i+1].c_str());
                _y[i] = atof(row[3*i+2].c_str());

                log_x[label[i]].push_back(_x[i]);
                log_y[label[i]].push_back(_y[i]);
                //std::cout << " " << row[i] << "\t";
            }
        }
        r++;
    }

    for(size_t i = 0; i < annotations; ++i){
        if(label_type[i] == "val" && val[i] > _threshold){
            segment(*(w_s[i]), in[i], _in, this->fps, label[i]);
            overlay(*(w_o[i]), in[i], _in, this->fps,
                    (float)_x[i]/(float)video_w+0.5*(float)rx[i]/(float)video_w,
                    (float)_y[i]/(float)video_h+0.5*(float)ry[i]/(float)video_h,
                    0.5*(float)rx[i]/(float)video_w,
                    0.5*(float)ry[i]/(float)video_h,
                    label[i]
                    );
        }
        else if(label_type[i] == "num" && num[i]!=" " && !num[i].empty() ){
            segment(*(w_s[i]), in[i], _in, this->fps, num[i]);
            overlay(*(w_o[i]), in[i], _in, this->fps,
                    (float)_x[i]/(float)video_w,
                    (float)_y[i]/(float)video_h,
                    0.05,
                    0.05,
                    num[i]
                    );
        }
        else if(label_type[i] == "txt" && txt[i]!=" "  && !txt[i].empty()){
            segment(*(w_s[i]), in[i], _in, this->fps, txt[i]);
            overlay(*(w_o[i]), in[i], _in, this->fps,
                    (float)_x[i]/(float)video_w,
                    (float)_y[i]/(float)video_h,
                    0.05,
                    0.05,
                    txt[i]
                    );
        }
        //std::cout << " " << row[i] << "\t";
    }

    for(size_t i = 0; i < annotations; ++i){

        overlayfooter(*(w_o[i]), label[i], "computer_vision",f, this->fps);
        segmentfooter(*(w_s[i]), label[i], "computer_vision",f, this->fps);

        //std::cout << s_o[i].GetString() << endl;
        std::cout << std::endl;

        std::ofstream overlayfile;
        std::string overlayfilepath = datapath + videostem + std::string("-") + label[i] + std::string("-overlays.json");
        overlayfile.open(overlayfilepath.c_str());

        if(overlayfile.is_open()){
            overlayfile << s_o[i].GetString();
            overlayfile.close();
        }

        //std::cout << s_s[i].GetString() << endl;
        std::cout << std::endl;

        std::ofstream segmentfile;
        std::string segmentfilepath = datapath + videostem + std::string("-") + label[i] + std::string("-segments.json");
        segmentfile.open(segmentfilepath.c_str());

        if(segmentfile.is_open()){
            segmentfile << s_s[i].GetString();
            segmentfile.close();
        }
    }

    stop = getTickCount();
    time = (double)(stop-start)/frequency;
    std::cout << "Time taken to parse csv and save jsons: " << time << std::endl;
    stop = start;

    for(size_t i = 0; i < annotations; ++i){
        if(label_type[i] == "time"){
            std::vector<int> _seg;
            std::vector<int> _min;
            int correct_seg_n = 0;
            int consecutive_correct_segs = 0;
            int first_correct_seg_id = -1;
            int __m = _m[i][0];
            int _c = 1;
            int _n_frames = _m[i].size();
            int _n_g_frames = 0;
            // Group minutes by successive values
            for(int c=1; c< _m[i].size(); c++ ){
                if (__m == _m[i][c]){
                    _c++;
                }
                if (__m != _m[i][c] || c == _m[i].size()-1){
                    _seg.push_back(_c);
                    _min.push_back(__m);
                    std::cout << "Found " << _c << " occurences of value " << __m << std::endl;
                    _n_g_frames += _c;

                    if( _c == fps*60){
                        correct_seg_n++;
                        if(first_correct_seg_id==-1){
                            first_correct_seg_id = _min.size()-1;
                        }
                    }
                    _c=1;
                }
                __m = _m[i][c];
            }
            std::cout << "Frames " << _n_frames << " grouped " << _n_g_frames << " in " << _min.size() << " groups " << std::endl;

            if(correct_seg_n == 0){
                std::cerr << "Couldn't find any properly sized segment of 1 minute of matching timestamps" << std::endl;
                return 0;
            }

            if(first_correct_seg_id == -1 || first_correct_seg_id >= _seg.size() ){
                std::cerr << "Couldn't locate the first properly sized segment of 1 minute of matching timestamps" << std::endl;
                return 0;
            }

            std::vector<int> consecutive_correct_seg_start;
            std::vector<int> consecutive_correct_seg_n;
            int _consecutive_correct_seg_start = first_correct_seg_id;
            int _consecutive_correct_seg_n = 0;
            //std::vector<int> _min;
            int _highest_consecutive_correct_seg_start = _consecutive_correct_seg_start;
            int _highest_consecutive_correct_seg_n = _consecutive_correct_seg_n;

            for(int s=first_correct_seg_id+1; s< _seg.size(); s++ ){
                //std::cout << "Occurences " << _seg[s] << " minutes now " << _min[s] << " was " << _min[s-1] <<" diff " << (_min[s] - _min[s-1])%60 << std::endl;
                bool incremental_consecutive = ( _seg[s] == _seg[s-1]) && (_seg[s] == fps*60 ) && ((_min[s] - _min[s-1])%60 == 1);
                if(incremental_consecutive){
                    if(_consecutive_correct_seg_n == 0){
                        _consecutive_correct_seg_start = s-1;
                    }
                    _consecutive_correct_seg_n++;
                }
                if( _consecutive_correct_seg_n>0 && (!incremental_consecutive || ( s == _seg.size()-1 && incremental_consecutive) )){
                    consecutive_correct_seg_start.push_back(_consecutive_correct_seg_start);
                    consecutive_correct_seg_n.push_back(_consecutive_correct_seg_n);

                    std::cout << "Found " << _consecutive_correct_seg_n << " consecutive occurences of incremental 1-minute timestamps from id " << _consecutive_correct_seg_start << " to " << s-1 << std::endl;

                    if(_consecutive_correct_seg_n > _highest_consecutive_correct_seg_n ){
                        _highest_consecutive_correct_seg_start = _consecutive_correct_seg_start;
                        _highest_consecutive_correct_seg_n = _consecutive_correct_seg_n;
                    }

                    //_consecutive_correct_seg_start = s;
                    _consecutive_correct_seg_n = 0;

                }
            }

            if(_highest_consecutive_correct_seg_n == 0){
                std::cerr << "Couldn't locate at least two consecutive properly sized segment of 1 minute of matching timestamps" << std::endl;
                return 0;
            }

            std::cout << "The highest consecutive occurences of incremental 1-minute timestamps amount to " << _highest_consecutive_correct_seg_n << "  from id " << _highest_consecutive_correct_seg_start << " to " << _highest_consecutive_correct_seg_start+_highest_consecutive_correct_seg_n << std::endl;

            std::deque<int> cseg;
            std::deque<int> cmin;

            int _cseg = 0;
            int _cmin = _min[_highest_consecutive_correct_seg_start];


            for(int s=_highest_consecutive_correct_seg_start-1; s>=0; s-- ){
                _cseg += _seg[s];
                if(_cseg >= fps*60){
                    _cmin--;
                    _cmin%=60;
                    cseg.push_front(fps*60);
                    cmin.push_front(_cmin);
                    _cseg -= fps*60;
                }
            }
            if(_cseg>0){
                _cmin--;
                _cmin%=60;
                cseg.push_front(_cseg);
                cmin.push_front(_cmin);
            }
            _cseg = 0;
            _cmin = _min[_highest_consecutive_correct_seg_start] -1 ;
            for(int s=_highest_consecutive_correct_seg_start; s<_seg.size(); s++ ){
                _cseg += _seg[s];
                if(_seg[s] == fps*60){
                    _cmin++;
                    _cmin%=60;
                    cseg.push_back(fps*60);
                    cmin.push_back(_cmin);
                    _cseg -= fps*60;
                }
            }
            while(_cseg>=0){
                _cmin++;
                _cmin%=60;
                cseg.push_back( (_cseg >= fps*60)? fps*60 : _cseg );
                cmin.push_back(_cmin);
                _cseg -= fps*60;
            }

            std::deque<int> chour;

            /*size_t timestempos = videostem.find_last_of("-");
            std::string timestem;
            if(timestempos!=std::string::npos){
                timestem = videostem.substr(timestempos+1);
            }
            std::cout << "Timestamp encoded in file name: '" << timestem << "'" << std::endl;


            int timestemh(0),timestemm(0);
            if(timestem.size() == 4){
                timestemh = atoi(timestem.substr(0,1).c_str())*10 + atoi(timestem.substr(1,1).c_str());
                timestemm = atoi(timestem.substr(2,1).c_str())*10 + atoi(timestem.substr(3,1).c_str());
            }

            std::cout << "Timestamp decoded from file name: " << timestemh << "h and " << timestemm << "m" << std::endl;

            int _timestemh(timestemh), _timestemm(timestemm);*/
            int _timestemh(start_t.h), _timestemm(start_t.m);

            int _n_c_frames = 0;
            int _i = 0;
            for(int s=0; s<cseg.size() && s<cmin.size(); s++ ){
                float _chour = 0.0;
                for(int __i = _i; __i < _i+cseg[s]; __i++){
                    _chour += (float)(_h[i][__i]);
                }
                _chour /= (float)cseg[s];
                chour.push_back( (int)_chour );
                _i+=cseg[s];
                _n_c_frames += cseg[s];

                std::cout << "Corrected " << cseg[s] << " occurrences of value " << _chour << ":" << cmin[s] << " to " << _timestemh << ":" << _timestemm << std::endl;

                _timestemm++;
                _timestemm%=60;
                if(_timestemm==0) _timestemh++;
                _timestemh%=24;
            }

            std::cout << "Frames " << _n_frames << " corrected " << _n_c_frames << std::endl;

            /*
            int begin_frame_var = _seg[0];
            int s=1;
            if(begin_frame_var<60*fps){
                while(s<_seg.size() && begin_frame_var+_seg[s]<60*fps ){
                    begin_frame_var += _seg[s];
                    s++;
                }
            }

            int end_frame_var = _seg[_seg.size()-1];
            s=_seg.size()-2;
            if(end_frame_var<60*fps){
                while(s>0 && end_frame_var+_seg[s]<60*fps){
                    end_frame_var += _seg[s];
                    s--;
                }
            }

            std::cout << "Frame mismatch at beginning " << cseg[0] << " vs " << begin_frame_var << " and " << cseg[cseg.size()-1] << " vs " << end_frame_var << std::endl;

            int _begin_frame_var = 0;
            for (int s=0; s<first_correct_seg_id; s++){
                _begin_frame_var += _seg[s];
            }
            _begin_frame_var %= 60*fps;
            int _end_frame_var = (_n_frames-_begin_frame_var) % (60*fps);

            std::cout << "Frame mismatch at beginning " << cseg[0] << " vs " << _begin_frame_var << " and " << cseg[cseg.size()-1] << " vs " << _end_frame_var << std::endl;
*/

            first_minute_frames = cseg[0];
            start_h = start_t.h;
            start_m = start_t.m;

            std::string timestampcsvpath = datapath + videostem + "_" + label[i] + "_Timestamp.csv";
            std::ofstream timestampcsvfile;
            timestampcsvfile.open(timestampcsvpath.c_str());
            if(!timestampcsvfile.is_open()){
                std::cerr << "Couldn't open log file '" << timestampcsvpath << "'" << std::endl;
                return 0;
            }
            timestampcsvfile << "\"Frame\",\"h\",\"m\",\"s\"" << std::endl;
            _i = 0;
            float __h = start_t.h;
            for(int s=0; s<cseg.size() && s<cmin.size(); s++ ){
                float _chour = 0.0;
                for(int __i = 0; __i < cseg[s]; __i++){
                    timestampcsvfile << _i+__i << "," << __h << "," << cmin[s] << "," << 60.0*(float)(fps*60 - cseg[s]+__i) / (float)(fps*60) << std::endl;
                }
                if(cmin[s] == 59){
                    __h++;
                }
                _i+=cseg[s];
            }
            timestampcsvfile.close();

            std::string timestamptxtpath = datapath + videostem + "_FirstFrameTimestamp.csv";
            std::ofstream timestamptxtfile;
            timestamptxtfile.open(timestamptxtpath.c_str());
            if(!timestamptxtfile.is_open()){
                std::cerr << "Couldn't open log file '" << timestamptxtpath << "'" << std::endl;
                return 0;
            }
            timestamptxtfile << "\"h\",\"m\",\"s\"" << std::endl;
            timestamptxtfile << start_t.h << "," << start_t.m << "," << 60.0*(float)(fps*60 - cseg[0]) / (float)(fps*60) << std::endl;
            timestamptxtfile.close();

            std::string firstminuteframestxtpath = datapath + videostem + "_FirstMinuteFrames.csv";
            std::ofstream firstminuteframestxtfile;
            firstminuteframestxtfile.open(firstminuteframestxtpath.c_str());
            if(!firstminuteframestxtfile.is_open()){
                std::cerr << "Couldn't open log file '" << firstminuteframestxtpath << "'" << std::endl;
                return 0;
            }
            firstminuteframestxtfile << "\"h\",\"m\",\"frames\"" << std::endl;
            firstminuteframestxtfile << start_t.h << "," << start_t.m << "," << cseg[0] << std::endl;
            firstminuteframestxtfile.close();
        }
    }

    return true;
}
