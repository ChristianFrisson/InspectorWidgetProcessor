# Doxyfile 1.8.3.1

#---------------------------------------------------------------------------
# Project related configuration options
#---------------------------------------------------------------------------
DOXYFILE_ENCODING      = UTF-8
PROJECT_NAME           = ${PROJECT_NAME}
PROJECT_NUMBER         = ${${PROJECT_NAME}_VERSION}
OUTPUT_DIRECTORY       = "${PROJECT_BINARY_DIR}/doc/"
CREATE_SUBDIRS         = NO
OUTPUT_LANGUAGE        = English
BRIEF_MEMBER_DESC      = YES
REPEAT_BRIEF           = YES
ABBREVIATE_BRIEF       = 
ALWAYS_DETAILED_SEC    = NO
INLINE_INHERITED_MEMB  = NO
FULL_PATH_NAMES        = YES
STRIP_FROM_PATH        = "${${PROJECT_NAME}_SOURCE_DIR}/"
STRIP_FROM_INC_PATH    = 
SHORT_NAMES            = YES
JAVADOC_AUTOBRIEF      = YES
QT_AUTOBRIEF           = YES
MULTILINE_CPP_IS_BRIEF = NO
INHERIT_DOCS           = YES
SEPARATE_MEMBER_PAGES  = NO
TAB_SIZE               = 8
ALIASES                = 
OPTIMIZE_OUTPUT_FOR_C  = NO
OPTIMIZE_OUTPUT_JAVA   = NO
OPTIMIZE_FOR_FORTRAN   = NO
OPTIMIZE_OUTPUT_VHDL   = NO
EXTENSION_MAPPING      =
MARKDOWN_SUPPORT       = YES
AUTOLINK_SUPPORT       = YES
BUILTIN_STL_SUPPORT    = NO
CPP_CLI_SUPPORT        = NO
SIP_SUPPORT            = NO
IDL_PROPERTY_SUPPORT   = YES
DISTRIBUTE_GROUP_DOC   = NO
SUBGROUPING            = YES
INLINE_GROUPED_CLASSES = NO
INLINE_SIMPLE_STRUCTS  = NO
TYPEDEF_HIDES_STRUCT   = NO
SYMBOL_CACHE_SIZE      = 0
LOOKUP_CACHE_SIZE      = 0
#---------------------------------------------------------------------------
# Build related configuration options
#---------------------------------------------------------------------------
EXTRACT_ALL            = YES
EXTRACT_PRIVATE        = NO
EXTRACT_PACKAGE        = NO
EXTRACT_STATIC         = NO
EXTRACT_LOCAL_CLASSES  = YES
EXTRACT_LOCAL_METHODS  = NO
EXTRACT_ANON_NSPACES   = NO
HIDE_UNDOC_MEMBERS     = NO
HIDE_UNDOC_CLASSES     = NO
HIDE_FRIEND_COMPOUNDS  = NO
HIDE_IN_BODY_DOCS      = NO
INTERNAL_DOCS          = NO
CASE_SENSE_NAMES       = YES
HIDE_SCOPE_NAMES       = NO
SHOW_INCLUDE_FILES     = YES
FORCE_LOCAL_INCLUDES   = NO
INLINE_INFO            = YES
SORT_MEMBER_DOCS       = YES
SORT_BRIEF_DOCS        = NO
SORT_MEMBERS_CTORS_1ST = NO
SORT_GROUP_NAMES       = NO
SORT_BY_SCOPE_NAME     = NO
STRICT_PROTO_MATCHING  = NO
GENERATE_TODOLIST      = YES
GENERATE_TESTLIST      = YES
GENERATE_BUGLIST       = YES
GENERATE_DEPRECATEDLIST= YES
ENABLED_SECTIONS       = 
MAX_INITIALIZER_LINES  = 30
SHOW_USED_FILES        = YES
SHOW_DIRECTORIES       = YES
SHOW_FILES             = YES
SHOW_NAMESPACES        = YES
FILE_VERSION_FILTER    =
LAYOUT_FILE            =
CITE_BIB_FILES         =
#---------------------------------------------------------------------------
# configuration options related to warning and progress messages
#---------------------------------------------------------------------------
QUIET                  = YES
WARNINGS               = YES
WARN_IF_UNDOCUMENTED   = YES
WARN_IF_DOC_ERROR      = YES
WARN_NO_PARAMDOC       = NO
WARN_FORMAT            = "$file:$line: $text"
WARN_LOGFILE           = 
#---------------------------------------------------------------------------
# configuration options related to the input files
#---------------------------------------------------------------------------
INPUT                  = ${${PROJECT_NAME}_SOURCE_DIR} \
                         ${PROJECT_BINARY_DIR} \
INPUT_ENCODING         = UTF-8
FILE_PATTERNS          = *ProcessorAPI.h 
RECURSIVE              = YES
EXCLUDE                = .git
EXCLUDE_SYMLINKS       = NO
EXCLUDE_PATTERNS       += */.git/*
EXCLUDE_PATTERNS       += 
EXCLUDE_PATTERNS       += 
EXCLUDE_SYMBOLS        = 
EXAMPLE_PATH           = 
EXAMPLE_PATTERNS       = 
EXAMPLE_RECURSIVE      = NO
IMAGE_PATH             = 
INPUT_FILTER           = 
FILTER_PATTERNS        = 
FILTER_SOURCE_FILES    = NO
FILTER_SOURCE_PATTERNS =
USE_MDFILE_AS_MAINPAGE =
#---------------------------------------------------------------------------
# configuration options related to source browsing
#---------------------------------------------------------------------------
SOURCE_BROWSER         = NO
INLINE_SOURCES         = NO
STRIP_CODE_COMMENTS    = YES
REFERENCED_BY_RELATION = YES
REFERENCES_RELATION    = YES
REFERENCES_LINK_SOURCE = YES
USE_HTAGS              = NO
VERBATIM_HEADERS       = YES
#---------------------------------------------------------------------------
# configuration options related to the alphabetical class index
#---------------------------------------------------------------------------
ALPHABETICAL_INDEX     = YES
COLS_IN_ALPHA_INDEX    = 2
IGNORE_PREFIX          = 
#---------------------------------------------------------------------------
# configuration options related to the HTML output
#---------------------------------------------------------------------------
GENERATE_HTML          = YES
HTML_OUTPUT            = html
HTML_FILE_EXTENSION    = .html
HTML_HEADER            = "${CMAKE_SOURCE_DIR}/doc/header.html"
HTML_FOOTER            = "${CMAKE_SOURCE_DIR}/doc/footer.html"
HTML_STYLESHEET        = "${CMAKE_SOURCE_DIR}/doc/customdoxygen.css"
HTML_ALIGN_MEMBERS     = YES
HTML_DYNAMIC_SECTIONS  = NO
HTML_INDEX_NUM_ENTRIES = 100
GENERATE_DOCSET        = NO
DOCSET_FEEDNAME        = "Doxygen generated docs"
DOCSET_BUNDLE_ID       = org.doxygen.Project
GENERATE_HTMLHELP      = ${GENERATE_HTMLHELP}
CHM_FILE               = "../${CMAKE_PROJECT_NAME}-${${PROJECT_NAME}_VERSION}.chm"
HHC_LOCATION           = "${HTML_HELP_COMPILER}"
GENERATE_CHI           = NO
CHM_INDEX_ENCODING     = 
BINARY_TOC             = NO
TOC_EXPAND             = NO
GENERATE_QHP           = NO
QCH_FILE               = 
QHP_NAMESPACE          = org.doxygen.Project
QHP_VIRTUAL_FOLDER     = doc
QHG_LOCATION           = 
DISABLE_INDEX          = NO
GENERATE_TREEVIEW      = YES
ENUM_VALUES_PER_LINE   = 4
TREEVIEW_WIDTH         = 250
EXT_LINKS_IN_WINDOW    = NO
FORMULA_FONTSIZE       = 10
FORMULA_TRANSPARENT    = YES
USE_MATHJAX            = NO
MATHJAX_FORMAT         = HTML-CSS
MATHJAX_RELPATH        = http://cdn.mathjax.org/mathjax/latest
MATHJAX_EXTENSIONS     =
SEARCHENGINE           = YES
SERVER_BASED_SEARCH    = NO
EXTERNAL_SEARCH        = NO
SEARCHENGINE_URL       =
SEARCHDATA_FILE        = searchdata.xml
EXTERNAL_SEARCH_ID     =
EXTRA_SEARCH_MAPPINGS  =
#---------------------------------------------------------------------------
# configuration options related to the LaTeX output
#---------------------------------------------------------------------------
GENERATE_LATEX         = ${GENERATE_LATEX}
LATEX_OUTPUT           = latex
LATEX_CMD_NAME         = latex
MAKEINDEX_CMD_NAME     = makeindex
COMPACT_LATEX          = NO
PAPER_TYPE             = a4
EXTRA_PACKAGES         =
LATEX_HEADER           =
LATEX_FOOTER           =
PDF_HYPERLINKS         = YES
USE_PDFLATEX           = YES
LATEX_BATCHMODE        = NO
LATEX_HIDE_INDICES     = NO
LATEX_SOURCE_CODE      = NO
LATEX_BIB_STYLE        = plain
#---------------------------------------------------------------------------
# configuration options related to the RTF output
#---------------------------------------------------------------------------
GENERATE_RTF           = NO
RTF_OUTPUT             = rtf
COMPACT_RTF            = NO
RTF_HYPERLINKS         = NO
RTF_STYLESHEET_FILE    = 
RTF_EXTENSIONS_FILE    = 
#---------------------------------------------------------------------------
# configuration options related to the man page output
#---------------------------------------------------------------------------
GENERATE_MAN           = NO
MAN_OUTPUT             = man
MAN_EXTENSION          = .3
MAN_LINKS              = NO
#---------------------------------------------------------------------------
# configuration options related to the XML output
#---------------------------------------------------------------------------
GENERATE_XML           = NO
XML_OUTPUT             = xml
XML_SCHEMA             = 
XML_DTD                = 
XML_PROGRAMLISTING     = YES
#---------------------------------------------------------------------------
# configuration options for the AutoGen Definitions output
#---------------------------------------------------------------------------
GENERATE_AUTOGEN_DEF   = NO
#---------------------------------------------------------------------------
# configuration options related to the Perl module output
#---------------------------------------------------------------------------
GENERATE_PERLMOD       = NO
PERLMOD_LATEX          = NO
PERLMOD_PRETTY         = YES
PERLMOD_MAKEVAR_PREFIX = 
#---------------------------------------------------------------------------
# Configuration options related to the preprocessor   
#---------------------------------------------------------------------------
ENABLE_PREPROCESSING   = YES
MACRO_EXPANSION        = NO
EXPAND_ONLY_PREDEF     = NO
SEARCH_INCLUDES        = YES
INCLUDE_PATH           = 
INCLUDE_FILE_PATTERNS  = 
PREDEFINED             = DOXYGEN_SHOULD_SKIP_THIS
EXPAND_AS_DEFINED      = 
SKIP_FUNCTION_MACROS   = YES
#---------------------------------------------------------------------------
# Configuration::additions related to external references   
#---------------------------------------------------------------------------
TAGFILES               = 
GENERATE_TAGFILE       = ${GENERATE_TAGFILE}
ALLEXTERNALS           = NO
EXTERNAL_GROUPS        = YES
PERL_PATH              = /usr/bin/perl
#---------------------------------------------------------------------------
# Configuration options related to the dot tool   
#---------------------------------------------------------------------------
CLASS_DIAGRAMS         = YES
MSCGEN_PATH            = 
HIDE_UNDOC_RELATIONS   = YES
HAVE_DOT               = YES
DOT_FONTNAME           = FreeSans
DOT_FONTSIZE           = 10
DOT_FONTPATH           = 
CLASS_GRAPH            = YES
COLLABORATION_GRAPH    = YES
GROUP_GRAPHS           = YES
UML_LOOK               = YES
TEMPLATE_RELATIONS     = YES
INCLUDE_GRAPH          = NO
INCLUDED_BY_GRAPH      = NO
CALL_GRAPH             = ${CALL_GRAPH}
CALLER_GRAPH           = ${CALLER_GRAPH}
GRAPHICAL_HIERARCHY    = YES
DIRECTORY_GRAPH        = YES
DOT_IMAGE_FORMAT       = png
DOT_PATH               = "${DOXYGEN_DOT_PATH}"
DOTFILE_DIRS           = 
DOT_GRAPH_MAX_NODES    = 100
MAX_DOT_GRAPH_DEPTH    = 0
DOT_TRANSPARENT        = YES
DOT_MULTI_TARGETS      = NO
GENERATE_LEGEND        = YES
DOT_CLEANUP            = NO
#---------------------------------------------------------------------------
# Configuration::additions related to the search engine   
#---------------------------------------------------------------------------
SEARCHENGINE           = ${SEARCHENGINE}