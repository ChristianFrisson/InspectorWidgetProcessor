/**
 * @file InspectorWidgetProcessor.h
 * @brief Tentative new API for InspectorWidgetProcessor
 * @author Christian Frisson
 */

#ifndef ProcessorAPI_H
#define ProcessorAPI_H

namespace InspectorWidget {

// Enum types not likely to be complemented anytime soon

/// Types for temporal annotations
enum AnnotationTemporalType{
    /// Undefined temporal annotation type
    ANNOTATION_TEMPORAL_NONE = 0,
    /// Event temporal annotation type
    ANNOTATION_TEMPORAL_EVENT = 1,
    /// Segment temporal annotation type
    ANNOTATION_TEMPORAL_SEGMENT = 2
};

/// Types for spatial annotations
enum AnnotationSpatialType{
    /// Undefined spatial annotation type
    ANNOTATION_SPATIAL_NONE = 0,
    /// Overlay spatial annotation type
    ANNOTATION_SPATIAL_OVERLAY = 1
};

// Enum types that might be complemented rarely

/// Types for annotation values
enum AnnotationValueType{
    /// Undefined annotation value type
    ANNOTATION_VALUE_NONE = 0,
    /// String annotation value type
    ANNOTATION_VALUE_STRING = 1,
    /// Float annotation value type
    ANNOTATION_VALUE_FLOAT = 2
};

// Enum types that might be complemented by plugins

/// Types for annotation sources
enum SourceType{
    /// Undefined annotation source type
    SOURCE_NONE = 0,
    /// Computer vision annotation source type
    SOURCE_CV = 1,
    /// Accessibility annotation source type
    SOURCE_AX = 2,
    /// Input hook annotation source type
    SOURCE_INPUT_HOOK = 3
};

/// Types for processing algorithms
enum ProcessorType{
    /// Default processing algorithm type
    PROCESSOR_NONE = 0,
    /// Processing algorithms using OpenCV
    PROCESSOR_OPENCV = 1
};

/// Types for export algorithms
enum ExportType{
    /// Undefined export algorithm type
    EXPORT_NONE = 0,
    /// Amalia.js export algorithm type
    EXPORT_AMALIA = 1
};

/// Types for files
enum FileType{
    /// Undefined file type
    FILE_NONE = 0,
    /// mp4 file type
    FILE_MP4 = 1,
    /// csv file type
    FILE_CSV = 2,
    /// txt file type
    FILE_TXT = 3,
    /// XML file type
    FILE_XML = 4,
    /// JSON file type
    FILE_JSON = 5
};

/// Annotation element
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

/// Annotation event with float values
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

/// Annotation event with string values
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

/// Annotation segment with string values
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

/// Annotation segment with float values
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

/// Annotation container
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

/// Source element
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

/// Source container
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

/// Video source
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

/// Collection container
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

/// Annotation definition
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

/// Pipeline definition
class Pipeline{
public:
    Pipeline(){}
    ~Pipeline(){}
    AnnotationDefinition* getDefinition(std::string name){return annotation_definitions[name];}
private:
    std::map<std::string,AnnotationDefinition*> annotation_definitions;
};

/// Analysis
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

/// Processor project definition
class Processor{
public:
    Processor(){}
    ~Processor(){}
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
