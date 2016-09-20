/**
 * @file InspectorWidgetProcessor.h
 * @brief Tentative new API for InspectorWidgetProcessor
 * @author Christian Frisson
 */

#ifndef InspectorWidgetProcessorAPI_H
#define InspectorWidgetProcessorAPI_H

#include <InspectorWidgetProcessor.h>

/// Enum types not likely to be complemented anytime soon
enum InspectorWidgetAnnotationTemporalType{
    INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_NONE = 0,
    INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_EVENT = 1,
    INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_SEGMENT = 2
};

enum InspectorWidgetAnnotationSpatialType{
    INSPECTOR_WIDGET_ANNOTATION_SPATIAL_NONE = 0,
    INSPECTOR_WIDGET_ANNOTATION_SPATIAL_OVERLAY = 1
};

/// Enum types that might be complemented rarely
enum InspectorWidgetAnnotationValueType{
    INSPECTOR_WIDGET_ANNOTATION_VALUE_NONE = 0,
    INSPECTOR_WIDGET_ANNOTATION_VALUE_STRING = 1,
    INSPECTOR_WIDGET_ANNOTATION_VALUE_FLOAT = 2
};

/// Enum types that might be complemented by plugins
enum InspectorWidgetSourceType{
    INSPECTOR_WIDGET_SOURCE_NONE = 0,
    INSPECTOR_WIDGET_SOURCE_CV = 1,
    INSPECTOR_WIDGET_SOURCE_AX = 2,
    INSPECTOR_WIDGET_SOURCE_INPUT_HOOK = 3
};

enum InspectorWidgetProcessorType{
    INSPECTOR_WIDGET_PROCESSOR_NONE = 0,
    INSPECTOR_WIDGET_PROCESSOR_OPENCV = 1
};

enum InspectorWidgetExportType{
    INSPECTOR_WIDGET_EXPORT_NONE = 0,
    INSPECTOR_WIDGET_EXPORT_AMALIA = 1
};

enum InspectorWidgetFileType{
    INSPECTOR_WIDGET_FILE_NONE = 0,
    INSPECTOR_WIDGET_FILE_MP4 = 1,
    INSPECTOR_WIDGET_FILE_CSV = 2,
    INSPECTOR_WIDGET_FILE_TXT = 3,
    INSPECTOR_WIDGET_FILE_XML = 4,
    INSPECTOR_WIDGET_FILE_JSON = 5
};


class InspectorWidgetAnnotationElement{
public:
    InspectorWidgetAnnotationElement(int time):time(time){}
    ~InspectorWidgetAnnotationElement(){}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_NONE;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_NONE;}
    int getTime(){return time;}
protected:
    int time;
};

class InspectorWidgetAnnotationFloatEvent : public InspectorWidgetAnnotationElement{
public:
    InspectorWidgetAnnotationFloatEvent(int time, float value)
        :InspectorWidgetAnnotationElement(time),value(value) {}
    ~InspectorWidgetAnnotationFloatEvent(){}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_EVENT;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_FLOAT;}
private:
    float value;
};

class InspectorWidgetAnnotationStringEvent : public InspectorWidgetAnnotationElement{
public:
    InspectorWidgetAnnotationStringEvent(int time, std::string value)
        :InspectorWidgetAnnotationElement(time),value(value) {}
    ~InspectorWidgetAnnotationStringEvent(){}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_EVENT;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_STRING;}
private:
    std::string value;
};

class InspectorWidgetAnnotationStringSegment : public InspectorWidgetAnnotationElement{
public:
    InspectorWidgetAnnotationStringSegment(int start_time, int stop_time, std::string value)
        :InspectorWidgetAnnotationElement(start_time),stop_time(stop_time),value(value) {}
    ~InspectorWidgetAnnotationStringSegment(){}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_SEGMENT;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_STRING;}
private:
    int stop_time;
    std::string value;
};

class InspectorWidgetAnnotationFloatSegment : public InspectorWidgetAnnotationElement{
public:
    InspectorWidgetAnnotationFloatSegment(int start_time, int stop_time, float value)
        :InspectorWidgetAnnotationElement(start_time),stop_time(stop_time),value(value) {}
    ~InspectorWidgetAnnotationFloatSegment(){}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_SEGMENT;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_FLOAT;}
private:
    int stop_time;
    float value;
};

class InspectorWidgetAnnotation{
public:
    InspectorWidgetAnnotation(){}
    virtual ~InspectorWidgetAnnotation(){this->clear();}

    virtual std::string getName(){return "";}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_NONE;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_NONE;}

    bool addElement(InspectorWidgetAnnotationElement* element){
        if(!element) return false;
        elements[element->getTime()] = element;
        return true;
    }
    InspectorWidgetAnnotationElement* getElement(int time){return elements[time];}
    void clear(){
        for(std::map<int,InspectorWidgetAnnotationElement*>::iterator element=elements.begin();element!=elements.end();element++){
            delete element->second;
        }
        elements.clear();
    }

private:
    std::map<int,InspectorWidgetAnnotationElement*> elements;
};

typedef std::map<std::string,InspectorWidgetAnnotation> InspectorWidgetAnnotations;

class InspectorWidgetSourceElement{
public:
    InspectorWidgetSourceElement(int time):time(time){}
    ~InspectorWidgetSourceElement(){}
    virtual InspectorWidgetSourceType sourceType(){return INSPECTOR_WIDGET_SOURCE_NONE;}
    virtual InspectorWidgetFileType fileType(){return INSPECTOR_WIDGET_FILE_NONE;}
    virtual InspectorWidgetProcessorType processorType(){return INSPECTOR_WIDGET_PROCESSOR_NONE;}
    int getTime(){return time;}
protected:
    int time;
};

class InspectorWidgetSource{
public:
    InspectorWidgetSource(){}
    ~InspectorWidgetSource(){}
    virtual InspectorWidgetSourceType sourceType(){return INSPECTOR_WIDGET_SOURCE_NONE;}
    virtual InspectorWidgetFileType fileType(){return INSPECTOR_WIDGET_FILE_NONE;}
    virtual bool open(){return false;}
    virtual bool close(){return false;}
    virtual bool load(){return false;}
    virtual bool unload(){return false;}
    std::map<int,InspectorWidgetSourceElement*> getElements(){return elements;}
    InspectorWidgetSourceElement* getElement(int time){return elements[time];}
    void clear(){
        for(std::map<int,InspectorWidgetSourceElement*>::iterator element=elements.begin();element!=elements.end();element++){
            delete element->second;
        }
        elements.clear();
    }
private:
    std::map<int,InspectorWidgetSourceElement*> elements;
};

class InspectorWidgetVideoSource: public InspectorWidgetSource{
public:
    InspectorWidgetVideoSource():InspectorWidgetSource(){}
    ~InspectorWidgetVideoSource(){}
    virtual InspectorWidgetSourceType sourceType(){return INSPECTOR_WIDGET_SOURCE_CV;}
    float getVideoFramerate(){return video_fps;}
    int getVideoWidth(){return video_w;}
    int getVideoHeight(){return video_h;}
    int getVideoFrameCount(){return video_frame_count;}
    InspectorWidgetDate getStartDate(){return start_d;}
    InspectorWidgetTime getStartTime(){return start_t;}
    InspectorWidgetTime getEndTime(){return end_t;}
private:
    float video_fps;
    int video_w;
    int video_h;
    int video_frame_count;
    InspectorWidgetTime start_t,end_t;
    InspectorWidgetDate start_d;
};

class InspectorWidgetCollection{
public:
    InspectorWidgetCollection()
        :video_fps(0),video_w(0),video_h(0),video_frame_count(0)
    {}
    ~InspectorWidgetCollection(){this->clear();}
    std::string getFileName(){return "";}
    InspectorWidgetDate getStartDate(){return start_d;}
    InspectorWidgetTime getStartTime(){return start_t;}
    virtual bool sourcesInit(){return false;}
    bool init(){
        video_fps = 0;
        video_w = 0;
        video_h = 0;
        video_frame_count = 0;
        start_t = InspectorWidgetTime();
        end_t = InspectorWidgetTime();
        start_d = InspectorWidgetDate();
        this->sourcesInit();
        InspectorWidgetSource* _source = 0;
        int video_sources = 0;
        for(std::map<std::string,InspectorWidgetSource*>::iterator source=sources.begin();source!=sources.end();source++){
            if(source->second->sourceType() == INSPECTOR_WIDGET_SOURCE_CV){
                video_sources++;
                _source = source->second;
            }
        }
        if(video_sources !=1){
            std::cerr << "Collections require a single video source, found: " << video_sources << std::endl;
            return false;
        }
        InspectorWidgetVideoSource* video_source = reinterpret_cast<InspectorWidgetVideoSource*>(_source);
        if(!video_source){
            std::cerr << "The video source found in collection is malformed" << std::endl;
            return false;
        }
        video_fps = video_source->getVideoFramerate();
        video_w = video_source->getVideoWidth();
        video_h = video_source->getVideoHeight();
        video_frame_count = video_source->getVideoFrameCount();
        start_t = video_source->getStartTime();
        end_t = video_source->getEndTime();
        start_d = video_source->getStartDate();
        if(video_fps == 0 ||
                video_w == 0 ||
                video_h == 0 ||
                video_frame_count == 0 ||
                start_t == InspectorWidgetTime()||
                end_t == InspectorWidgetTime()||
                start_d == InspectorWidgetDate()){
            std::cerr << "The video source found in collection has uninitialized parameters" << std::endl;
            return false;
        }
        return true;
    }
    void clear(){
        for(std::map<std::string,InspectorWidgetSource*>::iterator source=sources.begin();source!=sources.end();source++){
            delete source->second;
        }
        sources.clear();
    }
private:
    float video_fps;
    int video_w;
    int video_h;
    int video_frame_count;
    std::string path;
    InspectorWidgetTime start_t,end_t;
    InspectorWidgetDate start_d;
    std::map<std::string,InspectorWidgetSource*> sources;
};

class InspectorWidgetAnnotationDefinition{
public:
    InspectorWidgetAnnotationDefinition(){}
    ~InspectorWidgetAnnotationDefinition(){}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_NONE;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_NONE;}
    std::string getTest(){return test;}
    std::vector<std::string> getDependencies(){return dependencies;}
    std::string getAction(){return action;}
    std::vector<std::string> getVariables(){return variables;}
    bool hasTest(){return !test.empty();}
    bool hasDependencies(){return (dependencies.size()>0);}
    bool hasAction(){return !action.empty();}
    bool hasVariables(){return (variables.size()>0);}
private:
    std::string name;
    std::string test;
    std::vector<std::string> dependencies;
    std::string action;
    std::vector<std::string> variables;
};

class InspectorWidgetPipeline{
public:
    InspectorWidgetPipeline(){}
    ~InspectorWidgetPipeline(){}
    InspectorWidgetAnnotationDefinition* getDefinition(std::string name){return annotation_definitions[name];}
private:
    std::map<std::string,InspectorWidgetAnnotationDefinition*> annotation_definitions;
};

class InspectorWidgetAnalysis{
public:
    InspectorWidgetAnalysis(){}
    ~InspectorWidgetAnalysis(){}
private:
    InspectorWidgetPipeline  pipeline;
    InspectorWidgetCollection* collection;
    InspectorWidgetAnnotations annotations;
    int current_frame;
};

/// Algorithm to compute one annotation action
class InspectorWidgetSourceElementProcessor{
public:
    InspectorWidgetSourceElementProcessor(){}
    ~InspectorWidgetSourceElementProcessor(){}
    virtual std::string getName(){return "";}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_NONE;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_NONE;}
    virtual InspectorWidgetSourceType sourceType(){return INSPECTOR_WIDGET_SOURCE_NONE;}
    virtual InspectorWidgetProcessorType processorType(){return INSPECTOR_WIDGET_PROCESSOR_NONE;}
    virtual bool init(){return false;}
    virtual bool processElement(InspectorWidgetSourceElement* element){return false;}
    virtual std::string getAction()=0; // related to InspectorWidgetAnnotationDefinition
    virtual bool supportsDefinition(InspectorWidgetAnnotationDefinition definition)=0;
};

/// Algorithm to compute several annotation actions that share the same processor type
class InspectorWidgetSourceElementParser{
    InspectorWidgetSourceElementParser(){}
    ~InspectorWidgetSourceElementParser(){}
    virtual std::string getName(){return "";}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_NONE;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_NONE;}
    virtual InspectorWidgetSourceType sourceType(){return INSPECTOR_WIDGET_SOURCE_NONE;}
    virtual InspectorWidgetProcessorType processorType(){return INSPECTOR_WIDGET_PROCESSOR_NONE;}
    virtual bool init(){return false;}
    InspectorWidgetSourceElement* getNextElement(InspectorWidgetSourceElement* element){return 0;}
};

/// Algorithm to export annotations
class InspectorWidgetAnnotationExporter{
public:
    InspectorWidgetAnnotationExporter(){}
    ~InspectorWidgetAnnotationExporter(){}
    virtual std::string getName(){return "";}
    virtual InspectorWidgetAnnotationTemporalType temporalType(){return INSPECTOR_WIDGET_ANNOTATION_TEMPORAL_NONE;}
    virtual InspectorWidgetAnnotationValueType valueType(){return INSPECTOR_WIDGET_ANNOTATION_VALUE_NONE;}
    virtual InspectorWidgetSourceType sourceType(){return INSPECTOR_WIDGET_SOURCE_NONE;}
    virtual InspectorWidgetExportType exportType(){return INSPECTOR_WIDGET_EXPORT_NONE;}
    virtual bool openFile(){return false;}
    virtual bool closeFile(){return false;}
    virtual bool exportHeader(){return false;}
    virtual bool exportElement(InspectorWidgetAnnotationElement* element){return false;}
    virtual bool exportFooter(){return false;}
};

class InspectorWidgetProcessorNew{
public:
    InspectorWidgetProcessorNew(){}
    ~InspectorWidgetProcessorNew(){}
private:
    InspectorWidgetPipeline* pipeline;
    InspectorWidgetAnalysis* analysis;
    InspectorWidgetCollection* collection;
    std::map<std::string,InspectorWidgetSourceElementParser*> source_element_parsers;
    std::map<std::string,InspectorWidgetSourceElementProcessor*> source_element_processors; // mapped by action
    std::map<std::string,InspectorWidgetAnnotationExporter*> annotation_exporters;
};


#endif //InspectorWidgetProcessorAPI_H
