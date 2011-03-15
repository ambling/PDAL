/***************************************************************************
 *
 * Project: libLAS -- C/C++ read/write library for LAS LIDAR data
 * Purpose: LAS translation with optional configuration
 * Author:  Howard Butler, hobu.inc at gmail.com
 ***************************************************************************
 * Copyright (c) 2010, Howard Butler, hobu.inc at gmail.com 
 *
 * See LICENSE.txt in this source distribution for more information.
 **************************************************************************/


#include <iostream>

#include <libpc/exceptions.hpp>
//#include <libpc/libpc_config.hpp>
//#include <libpc/Bounds.hpp>
//#include <libpc/Color.hpp>
//#include <libpc/Dimension.hpp>
//#include <libpc/Schema.hpp>
//#include <libpc/CropFilter.hpp>
//#include <libpc/ColorFilter.hpp>
//#include <libpc/MosaicFilter.hpp>
//#include <libpc/FauxReader.hpp>
//#include <libpc/FauxWriter.hpp>
#include <libpc/drivers/las/Reader.hpp>
//#include <libpc/LasHeader.hpp>
#include <libpc/drivers/las/Writer.hpp>
#include <libpc/CacheFilter.hpp>

#include <libpc/drivers/liblas/writer.hpp>
#include <libpc/drivers/liblas/reader.hpp>

#ifdef HAVE_ORACLE
#include <libpc/drivers/oci/writer.hpp>
#endif

#include "Application.hpp"

using namespace libpc;
namespace po = boost::program_options;


class Application_pc2pc : public Application
{
public:
    Application_pc2pc(int argc, char* argv[]);
    int execute();

private:
    void addOptions();
    bool validateOptions();

    std::string m_inputFile;
    std::string m_outputFile;
};


Application_pc2pc::Application_pc2pc(int argc, char* argv[])
    : Application(argc, argv, "pc2pc")
{
}


bool Application_pc2pc::validateOptions()
{
    if (!hasOption("input"))
    {
        usageError("--input/-i required");
        return false;
    }

    if (!hasOption("output"))
    {
        usageError("--output/-o required");
        return false;
    }

    return true;
}


void Application_pc2pc::addOptions()
{
    po::options_description* file_options = new po::options_description("file options");

    file_options->add_options()
        ("input,i", po::value<std::string>(&m_inputFile), "input file name")
        ("output,o", po::value<std::string>(&m_outputFile), "output file name")
        ("native", "use native LAS classes (not liblas)")
        ("oracle", "oracle test")
        ;

    addOptionSet(file_options);
}

int Application_pc2pc::execute()
{
    if (!Utils::fileExists(m_inputFile))
    {
        runtimeError("file not found: " + m_inputFile);
        return 1;
    }

    std::istream* ifs = Utils::openFile(m_inputFile);
    std::ostream* ofs = Utils::createFile(m_outputFile);

    if (hasOption("native"))
    {
        LasReader reader(*ifs);
    
        const boost::uint64_t numPoints = reader.getHeader().getNumPoints();

        LasWriter writer(reader, *ofs);

        //BUG: handle laz writer.setCompressed(false);

        //writer.setPointFormat( reader.getPointFormatNumber() );

        size_t np = (size_t)numPoints;
        assert(numPoints == np); // BUG
        writer.write(np);
    }

#ifdef HAVE_ORACLE

    else if (hasOption("oracle"))
    {
#ifdef HAVE_ORACLE
        LiblasReader reader(*ifs);
    
        const boost::uint64_t numPoints = reader.getHeader().getNumPoints();
        
        libpc::driver::oci::Options options;
        boost::property_tree::ptree& tree = options.GetPTree();
        
        tree.put("capacity", 15);
        tree.put("connection", "lidar/lidar@oracle.hobu.biz/crrel");
        tree.put("debug", true);
        tree.put("verbose", true);
        
        CacheFilter cache(reader, 1, 1024);
        
        libpc::driver::oci::Writer writer(cache, options);
        
        
        //BUG: handle laz writer.setCompressed(false);

        //writer.setPointFormat( reader.getPointFormatNumber() );

        size_t np = (size_t)numPoints;
        assert(numPoints == np); // BUG
        writer.write(np);
#else
        throw configuration_error("libPC not compiled with Oracle support");
#endif
    }    
#endif
    else
    {
        LiblasReader reader(*ifs);
    
        const boost::uint64_t numPoints = reader.getHeader().getNumPoints();

        LiblasWriter writer(reader, *ofs);

        //BUG: handle laz writer.setCompressed(false);

        writer.setPointFormat( reader.getPointFormatNumber() );

        size_t np = (size_t)numPoints;
        assert(numPoints == np); // BUG
        writer.write(np);
    }

    Utils::closeFile(ofs);
    Utils::closeFile(ifs);

    return 0;
}

int main(int argc, char* argv[])
{
    Application_pc2pc app(argc, argv);
    return app.run();
}





#if 0

static void test1()
{
  // we are faking the reader, so we need to describe it here
  // the faux reader only supports fields (X,Y,Z,T)
  const Bounds<double> bounds(0, 0, -100, 200, 200, 100);
  const int numPoints = 30;
  FauxReader reader(bounds, numPoints, FauxReader::Random);

  CropFilter cropper(reader, Bounds<double>(0, 0, 0, 100, 100, 100));

  ColorFilter colorizer(cropper);

  FauxWriter writer(colorizer);
  
  writer.write(30);

  return;
}


static void test2()
{
  const int numPoints = 10;
  const Bounds<double> bounds1(0, 0, 0, 100, 100, 100);
  FauxReader reader1(bounds1, numPoints, FauxReader::Random);

  const Bounds<double> bounds2(100, 100, 100, 200, 200, 100);
  FauxReader reader2(bounds2, numPoints, FauxReader::Random);

  MosaicFilter mosaicker(reader1, reader2);

  FauxWriter writer(mosaicker);
  
  writer.write(20);
  
  return;
}



static void test3()
{
  std::istream* ifs = Utils::openFile("test/data/1.2-with-color.las");

  LasReader reader(*ifs);

  const Header& header = reader.getHeader();

  std::cout << (const LasHeader&)header;

  std::ostream* ofs = Utils::createFile("temp.las");

  LasWriter lasWriter(reader, *ofs);
  lasWriter.write( header.getNumPoints() );

  Utils::closeFile(ofs);

  Utils::closeFile(ifs);

  return;
}


int xmain(int, char* [])
{
    test3();

    {
        Bounds<double> bounds(1,2,3,4,5,6);
        std::cout << bounds << std::endl;
    }

    {
        const int major = GetVersionMajor();
        const int minor = GetVersionMinor();
        const int patch = GetVersionPatch();
        const int verInt = GetVersionInteger();
        std::cout << "Version: " << major << " " << minor << " " << patch << " (" << verInt << ")" << std::endl;

        const std::string verString = GetVersionString();
        std::cout << " string: " << verString << std::endl;

        const std::string fullString = GetFullVersionString();
        std::cout << "Full version string: " << fullString << std::endl;
    }

    {
        Dimension dim(Dimension::Field_User1, Dimension::Uint8);
        std::cout << dim << std::endl;
    }

    Color c;

    test1();

    test2();

    std::istream* ifs = Utils::openFile("test/data/1.2-with-color.las");
    // LiblasReader r(*ifs);

    return 0;
}

#endif

#if 0

#include <liblas/liblas.hpp>
#include "laskernel.hpp"

#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

namespace po = boost::program_options;

using namespace liblas;
using namespace std;

typedef boost::shared_ptr<liblas::Writer> WriterPtr;
typedef boost::shared_ptr<liblas::CoordinateSummary> SummaryPtr;


WriterPtr start_writer(   std::ostream*& ofs, 
                      std::string const& output, 
                      liblas::Header const& header)
{
ofs = liblas::Create(output, std::ios::out | std::ios::binary);
if (!ofs)
{
    std::ostringstream oss;
    oss << "Cannot create " << output << "for write.  Exiting...";
    throw std::runtime_error(oss.str());
}

WriterPtr writer( new liblas::Writer(*ofs, header));
return writer;
}

bool process(   std::istream& ifs,
                std::string const& output,
                liblas::Header & header,
                std::vector<liblas::FilterPtr>& filters,
                std::vector<liblas::TransformPtr>& transforms,
                boost::uint32_t split_mb,
                boost::uint32_t split_pts,
                bool verbose,
                bool min_offset)
{
    liblas::ReaderFactory f;
    liblas::Reader reader = f.CreateWithStream(ifs);
    SummaryPtr summary(new::liblas::CoordinateSummary);
    
    reader.SetFilters(filters);
    reader.SetTransforms(transforms);    
    
    if (min_offset) 
    {
        liblas::property_tree::ptree tree = SummarizeReader(reader);

        try
        {
            header.SetOffset(tree.get<double>("summary.points.minimum.x"),
                             tree.get<double>("summary.points.minimum.y"),
                             tree.get<double>("summary.points.minimum.z"));
    
                              
        }
        catch (liblas::property_tree::ptree_bad_path const& e) 
        {
            std::cerr << "Unable to write minimum header info.  Does the outputted file have any points?";
            std::cerr << e.what() << std::endl;
            return false;
        }
        if (verbose) 
        {
            std::cout << "Using minimum offsets ";
            SetStreamPrecision(std::cout, header.GetScaleX());
            std::cout << header.GetOffsetX() << " ";
            SetStreamPrecision(std::cout, header.GetScaleY());
            std::cout << header.GetOffsetY() << " ";
            SetStreamPrecision(std::cout, header.GetScaleZ());
            std::cout << header.GetOffsetZ() << " ";
            std::cout << std::endl;
        }
        reader.Reset();
    }

    std::ostream* ofs = NULL;
    std::string out = output;
    
    WriterPtr writer;

    if (!split_mb && !split_pts) {
        writer = start_writer(ofs, output, header);
        
    } else {
        string::size_type dot_pos = output.find_first_of(".");
        out = output.substr(0, dot_pos);
        writer = start_writer(ofs, out+"-1"+".las", header);
    }

    if (verbose)
    std::cout << "Writing output:" 
        << "\n - : " << output
        << std::endl;

    //
    // Translation of points cloud to features set
    //
    boost::uint32_t i = 0;
    boost::uint32_t const size = header.GetPointRecordsCount();
    
    boost::int32_t split_bytes_count = 1024*1024*split_mb;
    boost::uint32_t split_points_count = 0;
    int fileno = 2;

    while (reader.ReadNextPoint())
    {
        
        liblas::Point const& p = reader.GetPoint();
        summary->AddPoint(p);
        writer->WritePoint(p);
        if (verbose)
            term_progress(std::cout, (i + 1) / static_cast<double>(size));
        i++;
        split_points_count++;

        split_bytes_count = split_bytes_count - header.GetSchema().GetByteSize();        
        if (split_bytes_count < 0 && split_mb > 0 && ! split_pts) {
            // The user specifies a split size in mb, and we keep counting 
            // down until we've written that many points into the file.  
            // After that point, we make a new file and start writing into 
            // that.  

            // dereference the writer so it is deleted before the ofs
            writer = WriterPtr();
            
            delete ofs;
            ofs = NULL;

            ostringstream oss;
            oss << out << "-"<< fileno <<".las";

            writer = start_writer(ofs, oss.str(), header);

            ostringstream old_filename;
            old_filename << out << "-" << fileno - 1 << ".las";

            liblas::Header hnew = FetchHeader(old_filename.str());
            RepairHeader(*summary, hnew);
            RewriteHeader(hnew, old_filename.str());

            summary =  SummaryPtr(new liblas::CoordinateSummary); 
            fileno++;
            split_bytes_count = 1024*1024*split_mb;
        }

        if (split_pts > 0 && ! split_mb && split_points_count == split_pts) {
            // The user specifies a split size in pts, and we keep counting 
            // down until we've written that many points into the file.  
            // After that point, we make a new file and start writing into 
            // that.  

            // dereference the writer so it is deleted before the ofs
            writer = WriterPtr();
            
            delete ofs;
            ofs = NULL;

            ostringstream oss;
            oss << out << "-"<< fileno <<".las";

            writer = start_writer(ofs, oss.str(), header);

            ostringstream old_filename;
            old_filename << out << "-" << fileno - 1 << ".las";
            
            liblas::Header hnew = FetchHeader(old_filename.str());
            RepairHeader(*summary, hnew);
            RewriteHeader(hnew, old_filename.str());

            summary =  SummaryPtr(new liblas::CoordinateSummary); 
            fileno++;
            split_points_count = 0;
        }

    }
    if (verbose)
        std::cout << std::endl;

    // cheap hackery.  We need the Writer to disappear before the stream.  
    // Fix this up to not suck so bad.
    writer = WriterPtr();
    delete ofs;
    ofs = NULL;
        
    if (!split_mb && !split_pts) {
        reader.Reset();

        liblas::Header hnew = FetchHeader(output);
        RepairHeader(*summary, hnew);
        RewriteHeader(hnew, output);
    }

    return true;
}

void OutputHelp( std::ostream & oss, po::options_description const& options)
{
    oss << "--------------------------------------------------------------------\n";
    oss << "    las2las (" << GetFullVersion() << ")\n";
    oss << "--------------------------------------------------------------------\n";

    oss << options;

    oss <<"\nFor more information, see the full documentation for las2las at:\n";
    
    oss << " http://liblas.org/utilities/las2las.html\n";
    oss << "----------------------------------------------------------\n";

}

int main(int argc, char* argv[])
{

    boost::uint32_t split_mb = 0;
    boost::uint32_t split_pts = 0;
    std::string input;
    std::string output;
    std::string output_format;
    
    bool verbose = false;
    bool bMinOffset = false;
    
    std::vector<liblas::FilterPtr> filters;
    std::vector<liblas::TransformPtr> transforms;

    bool bCompressed = false;
    liblas::Header header;

    try {

        po::options_description file_options("las2las options");
        po::options_description filtering_options = GetFilteringOptions();
        po::options_description header_options = GetHeaderOptions();
        po::options_description transform_options = GetTransformationOptions() ;

        po::positional_options_description p;
        p.add("input", 1);
        p.add("output", 1);

        file_options.add_options()
            ("help,h", "produce help message")
            ("split-mb", po::value<boost::uint32_t>(&split_mb)->default_value(0), "Split file into multiple files with each being this size in MB or less. If this value is 0, no splitting is done")
            ("split-pts", po::value<boost::uint32_t>(&split_pts)->default_value(0), "Split file into multiple files with each being this many points or less. If this value is 0, no splitting is done")
            ("input,i", po::value< string >(), "input LAS file")
            ("output,o", po::value< string >(&output)->default_value("output.las"), "output LAS file")
            ("compressed,c", po::value<bool>(&bCompressed)->zero_tokens()->implicit_value(true), "Produce .laz compressed data")
            ("verbose,v", po::value<bool>(&verbose)->zero_tokens(), "Verbose message output")
        ;

        po::variables_map vm;
        po::options_description options;
        options.add(file_options).add(header_options).add(transform_options).add(filtering_options);
        po::store(po::command_line_parser(argc, argv).
          options(options).positional(p).run(), vm);

        po::notify(vm);

        if (vm.count("help")) 
        {
            OutputHelp(std::cout, options);
            return 1;
        }

        if (split_pts > 0 && split_mb > 0) 
        {
            std::cerr << "Both split-mb and split-pts cannot be used simultaneously." << std::endl; 
            return 1;
        }

        if (vm.count("input")) 
        {
            input = vm["input"].as< string >();
            
            if (verbose)
                std::cout << "Opening " << input << " to fetch Header" << std::endl;

            std::istream* ifs = Open(input, std::ios::in | std::ios::binary);
            if (!ifs)
            {
                std::cerr << "Cannot open " << input << " for read.  Exiting..." << std::endl;
                return 1;
            }
            // set_ifstream_buffer(ifs, default_buffer_size);

            {// scope the reader, so it goes away before the stream does
                liblas::ReaderFactory f;
                liblas::Reader reader = f.CreateWithStream(*ifs);
                header = reader.GetHeader();
            }
            delete ifs;
        } else {
            std::cerr << "Input LAS file not specified!\n";
            OutputHelp(std::cout, options);
            return 1;
        }

        if (vm.count("min-offset")) 
        {
            if (vm.count("offset")) {
                throw std::runtime_error("min-offset cannot be used with offets.  Use one or the other");
            }
            
            bMinOffset = true;
        } 

        filters = GetFilters(vm, verbose);
        
        // Transforms alter our header as well.  Setting scales, offsets, etc.
        transforms = GetTransforms(vm, verbose, header);

        if (bCompressed)
        {
            header.SetCompressed(true);
        }
        else 
        {
            SetHeaderCompression(header, output);
        }

        std::istream* ifs = Open(input, std::ios::in | std::ios::binary);
        if (!ifs)
        {
            std::cerr << "Cannot open " << input << " for read.  Exiting..." << std::endl;
            return 1;
        }

        bool op = process(  *ifs, 
                            output,
                            header, 
                            filters,
                            transforms,
                            split_mb,
                            split_pts,
                            verbose,
                            bMinOffset
                            );
        if (!op) {
            return (1);
        }
        
        if (ifs != 0)
        {
            liblas::Cleanup(ifs);
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...)
    {
        std::cerr << "Exception of unknown type!\n";
    }
    
    return 0;


}

//las2las -i lt_srs_rt.las  -o foo.las -c 1,2 -b 2483590,366208,2484000,366612
#endif
