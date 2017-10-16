#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkConfidenceConnectedImageFilter.h"
#include "itksys/SystemTools.hxx"
#include <sstream>
// #include "QuickView.h"

// DICOM series
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageSeriesWriter.h"
#include <vector>
#include "itksys/SystemTools.hxx"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "gdcmUIDGenerator.h"
#include <sstream>
#include "itkTimeProbe.h"
#include "itkPNGImageIO.h"
#include "itkStatisticsImageFilter.h"

// string
#include <string>
#include <cstdlib>

typedef itk::Image< unsigned char, 2 >  ImageType;

int main( int argc, char *argv[])
{
  if(argc != 6)
  {
    std::cerr << "Usage: " << argv[0] << "inputDir outputDir seedX seedY seedZ" << std::endl;
    return EXIT_FAILURE;
  } else {
    std::cerr << std::endl;
		std::cerr << "Input arguments:" << std::endl;
		std::cerr << "Executable: " << argv[0] << std::endl;
		std::cerr << "Input Dicom Directory: " << argv[1] << std::endl;
		std::cerr << "Output Dicom Directory: " << argv[2] << std::endl;
    std::cerr << "Input Seed Coordinate X: " << argv[3] << std::endl;
    std::cerr << "Input Seed Coordinate Y: " << argv[4] << std::endl;
    std::cerr << "Input Seed Coordinate Z: " << argv[5] << std::endl;
  }

  /* ============================== Type & Filter Definitions ============================== */
  typedef float InternalPixelType;
	const unsigned int InputDimension = 3;
	typedef itk::Image< InternalPixelType, InputDimension > InternalImageType_3D;
	typedef itk::ImageSeriesReader< InternalImageType_3D > ReaderType;
	typedef itk::GDCMImageIO							 ImageIOType;
	typedef itk::GDCMSeriesFileNames					 NamesGeneratorType;

  //define output filter for writing series as DICOM images
	typedef signed short OutputPixelType;
	const unsigned int OutputDimension = 2;
	typedef itk::Image< OutputPixelType, OutputDimension > OutputImageType_2D;
	typedef itk::ImageSeriesWriter< InternalImageType_3D, OutputImageType_2D > SeriesWriterType;

  //define vectors for storing image sizes, etc.
	typedef itk::Vector<double, 3> VectorType;
	typedef itk::Vector<int, 3> VectorTypeInt;
	std::vector<float> maskPixelValues_shrunk; //vector to store pixel values corresponding to mask == 1

  /* ============================== Read DICOM Images ============================== */
	std::cerr << "Reading images..." << std::endl;
	std::cerr << std::endl;

	//get and check # of input files & filenames
	NamesGeneratorType::Pointer namesGenerator = NamesGeneratorType::New();
	namesGenerator->SetInputDirectory( argv[1] );
	const ReaderType::FileNamesContainer & inputFilenames = namesGenerator->GetInputFileNames();
	unsigned int numberOfInputFilenames =  inputFilenames.size();
	std::cerr << "Total # of files in Input Dicom Directory: " << numberOfInputFilenames << std::endl;

  std::cerr << std::endl;
	if(numberOfInputFilenames == 0)
		return EXIT_FAILURE;

	//read in images
	ImageIOType::Pointer gdcmIO = ImageIOType::New();
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetImageIO( gdcmIO );
	reader->SetFileNames( inputFilenames );
	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject &excp)
	{
		std::cerr << "Exception thrown while reading the series!" << std::endl;
		std::cerr << excp << std::endl;
		return EXIT_FAILURE;
	}

  //define image properties (e.g., size and dimensions) to write out images correctly later
  InternalImageType_3D::RegionType inputRegion;
  InternalImageType_3D::IndexType inputImageIndex = reader->GetOutput()->GetLargestPossibleRegion().GetIndex();
  InternalImageType_3D::SizeType inputImageSize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
  inputRegion.SetIndex( inputImageIndex );
  inputRegion.SetSize( inputImageSize );

  typedef itk::ConfidenceConnectedImageFilter<InternalImageType_3D, InternalImageType_3D> ConfidenceConnectedFilterType;
  ConfidenceConnectedFilterType::Pointer confidenceConnectedFilter = ConfidenceConnectedFilterType::New();
  confidenceConnectedFilter->SetInitialNeighborhoodRadius(3);
  confidenceConnectedFilter->SetMultiplier(3);
  confidenceConnectedFilter->SetNumberOfIterations(2);
  confidenceConnectedFilter->SetReplaceValue(3000);

  // convert seed point
  typedef itk::MetaDataDictionary                  DictionaryType;
  typedef itk::MetaDataDictionary *                DictionaryRawPointer;
  typedef std::vector< DictionaryRawPointer > DictionaryArrayType;
  typedef const DictionaryArrayType *         DictionaryArrayRawPointer;

  DictionaryArrayRawPointer dictionaryArrayPointer;
  dictionaryArrayPointer = reader->GetMetaDataDictionaryArray();

  typedef itk::MetaDataObject< std::string > MetaDataStringType;
  std::string keyToSearch = "0020|0013";
  int frontEndIndex_z = atoi(argv[5]);
  int countIndexZ = 0;
  int itkIndexZ = 0;
  bool breakWhileLoop = false;
  for( DictionaryArrayType::const_iterator it = dictionaryArrayPointer->begin(); it != dictionaryArrayPointer->end(); ++it) {
    DictionaryRawPointer drp = *it;
    DictionaryType::ConstIterator itr = drp->Begin();
    DictionaryType::ConstIterator end = drp->End();
    while( itr != end ){
      itk::MetaDataObjectBase::Pointer entry = itr->second;
      MetaDataStringType::Pointer entryvalue = dynamic_cast<MetaDataStringType *>( entry.GetPointer() );

      if( entryvalue ){
        std::string tagkey = itr->first;
        std::string tagvalue = entryvalue->GetMetaDataObjectValue();
        if (keyToSearch.compare(tagkey) == 0){
          if (frontEndIndex_z == atoi(tagvalue.c_str())) {
            std::cout << tagkey <<  " = " << tagvalue << std::endl;
            std::cout << "===============" << std::endl;
            breakWhileLoop = true;
            itkIndexZ = countIndexZ;
            break;
          }
          countIndexZ++;
        }
      }
      if(breakWhileLoop){
        break;
      }
      ++itr;
    }
	}

  // Set seed
  InternalImageType_3D::IndexType seed;
  seed[0] = atoi(argv[3]);
  seed[1] = atoi(argv[4]);
  seed[2] = itkIndexZ;
  std::cerr << "Final Seed Coordinate X: " << seed[0] << std::endl;
  std::cerr << "Final Seed Coordinate Y: " << seed[1] << std::endl;
  std::cerr << "Final Seed Coordinate Z: " << seed[2] << std::endl;
  confidenceConnectedFilter->SetSeed(seed);
  confidenceConnectedFilter->SetInput(reader->GetOutput());
  confidenceConnectedFilter->Update();
  InternalImageType_3D::Pointer segmentationLabelMap = confidenceConnectedFilter->GetOutput();

  //write out the bias corrected image
  itksys::SystemTools::MakeDirectory( argv[2] );
  SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();
  seriesWriter->SetInput( segmentationLabelMap );

  namesGenerator->SetOutputDirectory( argv[2] );
  const ReaderType::FileNamesContainer & outputFilenames = namesGenerator->GetOutputFileNames();

  unsigned int numberOfOutputFilenames =  outputFilenames.size();
  if(numberOfOutputFilenames == 0 || numberOfOutputFilenames != numberOfInputFilenames)
    return EXIT_FAILURE;
  seriesWriter->SetFileNames( outputFilenames );
  seriesWriter->SetImageIO( gdcmIO );
  seriesWriter->SetMetaDataDictionaryArray( reader->GetMetaDataDictionaryArray() );
  try
  {
    seriesWriter->Update();
  }
  catch( itk::ExceptionObject & excp )
  {
    std::cerr << "Exception thrown while writing the series!" << std::endl;
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Simple Region Growing is done." << std::endl;
  return EXIT_SUCCESS;
}
