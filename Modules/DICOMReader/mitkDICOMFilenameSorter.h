/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkDICOMFilenameSorter_h
#define mitkDICOMFilenameSorter_h

#include "mitkDICOMDatasetSorter.h"

namespace mitk
{

/**
  \brief sort files based on filename (last resort).
*/
class DICOMReader_EXPORT DICOMFilenameSorter : public DICOMDatasetSorter
{
  public:

    mitkClassMacro( DICOMFilenameSorter, DICOMDatasetSorter )
    itkNewMacro( DICOMFilenameSorter )

    virtual DICOMTagList GetTagsOfInterest();

    virtual void Sort();

  protected:

    struct FilenameSort
    {
      bool operator() (const mitk::DICOMDatasetAccess* left, const mitk::DICOMDatasetAccess* right);
    };

    DICOMFilenameSorter();
    virtual ~DICOMFilenameSorter();

    DICOMFilenameSorter(const DICOMFilenameSorter& other);
    DICOMFilenameSorter& operator=(const DICOMFilenameSorter& other);
};

}

#endif
