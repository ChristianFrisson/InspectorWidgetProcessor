/**
 * @file InspectorWidgetProcessor.h
 * @brief Tentative new API for InspectorWidgetProcessor
 * @author Christian Frisson
 */

#ifndef ProcessorAPI_H
#define ProcessorAPI_H

namespace InspectorWidget {

/// Enum types not likely to be complemented anytime soon
enum AnnotationTemporalType{
    ANNOTATION_TEMPORAL_NONE = 0,
    ANNOTATION_TEMPORAL_EVENT = 1,
    ANNOTATION_TEMPORAL_SEGMENT = 2
};

enum AnnotationSpatialType{
    ANNOTATION_SPATIAL_NONE = 0,
    ANNOTATION_SPATIAL_OVERLAY = 1
};

/// Enum types that might be complemented rarely
enum AnnotationValueType{
    ANNOTATION_VALUE_NONE = 0,
    ANNOTATION_VALUE_STRING = 1,
    ANNOTATION_VALUE_FLOAT = 2
};

/// Enum types that might be complemented by plugins
enum SourceType{
    SOURCE_NONE = 0,
    SOURCE_CV = 1,
    SOURCE_AX = 2,
    SOURCE_INPUT_HOOK = 3
};

enum ProcessorType{
    PROCESSOR_NONE = 0,
    PROCESSOR_OPENCV = 1
};

enum ExportType{
    EXPORT_NONE = 0,
    EXPORT_AMALIA = 1
};

enum FileType{
    FILE_NONE = 0,
    FILE_MP4 = 1,
    FILE_CSV = 2,
    FILE_TXT = 3,
    FILE_XML = 4,
    FILE_JSON = 5
};


class AnnotationElement{
public:
    AnnotationElement(int time):time(time){}
    ~AnnotationElement(){}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_NONE;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_NONE;}
    int getTime(){return time;}
protected:
    int time;
};

class AnnotationFloatEvent : public AnnotationElement{
public:
    AnnotationFloatEvent(int time, float value)
        :AnnotationElement(time),value(value) {}
    ~AnnotationFloatEvent(){}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_EVENT;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_FLOAT;}
private:
    float value;
};

class AnnotationStringEvent : public AnnotationElement{
public:
    AnnotationStringEvent(int time, std::string value)
        :AnnotationElement(time),value(value) {}
    ~AnnotationStringEvent(){}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_EVENT;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_STRING;}
private:
    std::string value;
};

class AnnotationStringSegment : public AnnotationElement{
public:
    AnnotationStringSegment(int start_time, int stop_time, std::string value)
        :AnnotationElement(start_time),stop_time(stop_time),value(value) {}
    ~AnnotationStringSegment(){}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_SEGMENT;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_STRING;}
private:
    int stop_time;
    std::string value;
};

class AnnotationFloatSegment : public AnnotationElement{
public:
    AnnotationFloatSegment(int start_time, int stop_time, float value)
        :AnnotationElement(start_time),stop_time(stop_time),value(value) {}
    ~AnnotationFloatSegment(){}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_SEGMENT;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_FLOAT;}
private:
    int stop_time;
    float value;
};

class Annotation{
public:
    Annotation(){}
    virtual ~Annotation(){this->clear();}

    virtual std::string getName(){return "";}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_NONE;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_NONE;}

    bool addElement(AnnotationElement* element){
        if(!element) return false;
        elements[element->getTime()] = element;
        return true;
    }
    AnnotationElement* getElement(int time){return elements[time];}
    void clear(){
        for(std::map<int,AnnotationElement*>::iterator element=elements.begin();element!=elements.end();element++){
            delete element->second;
        }
        elements.clear();
    }

private:
    std::map<int,AnnotationElement*> elements;
};

typedef std::map<std::string,Annotation> Annotations;

class SourceElement{
public:
    SourceElement(int time):time(time){}
    ~SourceElement(){}
    virtual SourceType sourceType(){return SOURCE_NONE;}
    virtual FileType fileType(){return FILE_NONE;}
    virtual ProcessorType processorType(){return PROCESSOR_NONE;}
    int getTime(){return time;}
protected:
    int time;
};

class Source{
public:
    Source(){}
    ~Source(){}
    virtual SourceType sourceType(){return SOURCE_NONE;}
    virtual FileType fileType(){return FILE_NONE;}
    virtual bool open(){return false;}
    virtual bool close(){return false;}
    virtual bool load(){return false;}
    virtual bool unload(){return false;}
    std::map<int,SourceElement*> getElements(){return elements;}
    SourceElement* getElement(int time){return elements[time];}
    void clear(){
        for(std::map<int,SourceElement*>::iterator element=elements.begin();element!=elements.end();element++){
            delete element->second;
        }
        elements.clear();
    }
private:
    std::map<int,SourceElement*> elements;
};

class VideoSource: public Source{
public:
    VideoSource():Source(){}
    ~VideoSource(){}
    virtual SourceType sourceType(){return SOURCE_CV;}
    float getVideoFramerate(){return video_fps;}
    int getVideoWidth(){return video_w;}
    int getVideoHeight(){return video_h;}
    int getVideoFrameCount(){return video_frame_count;}
    Date getStartDate(){return start_d;}
    Time getStartTime(){return start_t;}
    Time getEndTime(){return end_t;}
private:
    float video_fps;
    int video_w;
    int video_h;
    int video_frame_count;
    Time start_t,end_t;
    Date start_d;
};

class Collection{
public:
    Collection()
        :video_fps(0),video_w(0),video_h(0),video_frame_count(0)
    {}
    ~Collection(){this->clear();}
    std::string getFileName(){return "";}
    Date getStartDate(){return start_d;}
    Time getStartTime(){return start_t;}
    virtual bool sourcesInit(){return false;}
    bool init(){
        video_fps = 0;
        video_w = 0;
        video_h = 0;
        video_frame_count = 0;
        start_t = Time();
        end_t = Time();
        start_d = Date();
        this->sourcesInit();
        Source* _source = 0;
        int video_sources = 0;
        for(std::map<std::string,Source*>::iterator source=sources.begin();source!=sources.end();source++){
            if(source->second->sourceType() == SOURCE_CV){
                video_sources++;
                _source = source->second;
            }
        }
        if(video_sources !=1){
            std::cerr << "Collections require a single video source, found: " << video_sources << std::endl;
            return false;
        }
        VideoSource* video_source = reinterpret_cast<VideoSource*>(_source);
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
                start_t == Time()||
                end_t == Time()||
                start_d == Date()){
            std::cerr << "The video source found in collection has uninitialized parameters" << std::endl;
            return false;
        }
        return true;
    }
    void clear(){
        for(std::map<std::string,Source*>::iterator source=sources.begin();source!=sources.end();source++){
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
    Time start_t,end_t;
    Date start_d;
    std::map<std::string,Source*> sources;
};

class AnnotationDefinition{
public:
    AnnotationDefinition(){}
    ~AnnotationDefinition(){}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_NONE;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_NONE;}
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

class Pipeline{
public:
    Pipeline(){}
    ~Pipeline(){}
    AnnotationDefinition* getDefinition(std::string name){return annotation_definitions[name];}
private:
    std::map<std::string,AnnotationDefinition*> annotation_definitions;
};

class Analysis{
public:
    Analysis(){}
    ~Analysis(){}
private:
    Pipeline  pipeline;
    Collection* collection;
    Annotations annotations;
    int current_frame;
};

/// Algorithm to compute one annotation action
class SourceElementProcessor{
public:
    SourceElementProcessor(){}
    ~SourceElementProcessor(){}
    virtual std::string getName(){return "";}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_NONE;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_NONE;}
    virtual SourceType sourceType(){return SOURCE_NONE;}
    virtual ProcessorType processorType(){return PROCESSOR_NONE;}
    virtual bool init(){return false;}
    virtual bool processElement(SourceElement* element){return false;}
    virtual std::string getAction()=0; // related to AnnotationDefinition
    virtual bool supportsDefinition(AnnotationDefinition definition)=0;
};

/// Algorithm to compute several annotation actions that share the same processor type
class SourceElementParser{
    SourceElementParser(){}
    ~SourceElementParser(){}
    virtual std::string getName(){return "";}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_NONE;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_NONE;}
    virtual SourceType sourceType(){return SOURCE_NONE;}
    virtual ProcessorType processorType(){return PROCESSOR_NONE;}
    virtual bool init(){return false;}
    SourceElement* getNextElement(SourceElement* element){return 0;}
};

/// Algorithm to export annotations
class AnnotationExporter{
public:
    AnnotationExporter(){}
    ~AnnotationExporter(){}
    virtual std::string getName(){return "";}
    virtual AnnotationTemporalType temporalType(){return ANNOTATION_TEMPORAL_NONE;}
    virtual AnnotationValueType valueType(){return ANNOTATION_VALUE_NONE;}
    virtual SourceType sourceType(){return SOURCE_NONE;}
    virtual ExportType exportType(){return EXPORT_NONE;}
    virtual bool openFile(){return false;}
    virtual bool closeFile(){return false;}
    virtual bool exportHeader(){return false;}
    virtual bool exportElement(AnnotationElement* element){return false;}
    virtual bool exportFooter(){return false;}
};

class ProcessorNew{
public:
    ProcessorNew(){}
    ~ProcessorNew(){}
private:
    Pipeline* pipeline;
    Analysis* analysis;
    Collection* collection;
    std::map<std::string,SourceElementParser*> source_element_parsers;
    std::map<std::string,SourceElementProcessor*> source_element_processors; // mapped by action
    std::map<std::string,AnnotationExporter*> annotation_exporters;
};

}

#endif //ProcessorAPI_H
