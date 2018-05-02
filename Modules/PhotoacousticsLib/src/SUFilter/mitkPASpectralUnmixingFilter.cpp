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

#include "mitkPASpectralUnmixingFilter.h"

#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

// Test with ImagePixelAccessor
#include <mitkImagePixelAccessor.h>
#include <mitkImagePixelWriteAccessor.h>

mitk::pa::SpectralUnmixingFilter::SpectralUnmixingFilter()
{
  this->SetNumberOfIndexedOutputs(2);

  for (unsigned int i = 0; i<GetNumberOfIndexedOutputs(); i++)
  {
    this->SetNthOutput(i, mitk::Image::New());
  }

  m_PropertyCalculator = mitk::pa::PropertyCalculator::New();
}

mitk::pa::SpectralUnmixingFilter::~SpectralUnmixingFilter()
{

}

void mitk::pa::SpectralUnmixingFilter::AddWavelength(int wavelength)
{
  m_Wavelength.push_back(wavelength);
}

void mitk::pa::SpectralUnmixingFilter::SetChromophores(ChromophoreType chromophore)
{
  m_Chromophore.push_back(chromophore);
}

void mitk::pa::SpectralUnmixingFilter::GenerateData()
{
  MITK_INFO << "GENERATING DATA..";

  unsigned int xDim = m_Dimensions[0];
  unsigned int yDim = m_Dimensions[1];
  unsigned int zDim = m_Dimensions[2];

  MITK_INFO << "NumberOfInputImages: " << zDim;

  CheckPreConditions(zDim);

  InitializeOutputs();

  mitk::Image::Pointer data = GetInput();

  for (unsigned int x = 0; x < xDim; x++)
  {
    for (unsigned int y = 0; y < yDim; y++)
    {
      Eigen::VectorXd inputVector(numberOfInputs);

      for (unsigned int z = 0; z < zDim; z++)
      {
        //inputVector[z] = wertvonpixelmitderwellenlänge;
        

      }
      
      Eigen::VectorXd resultVector = SpectralUnmixingAlgorithms(inputVector);

      for (int outputIdx = 0; outputIdx < GetNumberOfIndexedOutputs(); ++outputIdx)
      {
        auto output = GetOutput(outputIdx);
        mitk::ImageWriteAccessor writeOutput(output);
        float* writeBuffer = (float *)writeOutput.GetData();

        writeBuffer[y*xDim + x] = resultVector[outputIdx];
      }

    }
  }
  MITK_INFO << "GENERATING DATA...[DONE]";
}

// creats vector with x, y, z dimensions as entries
void mitk::pa::SpectralUnmixingFilter::SetDimensions(int Dimension)
{
  m_Dimensions.push_back(Dimension);
}

// checks if number of Inputs == added wavelengths 
void mitk::pa::SpectralUnmixingFilter::CheckPreConditions(unsigned int NumberOfInputImages)
{
  if (m_Wavelength.size() != NumberOfInputImages)
    mitkThrow() << "CHECK INPUTS! WAVELENGTHERROR";
  MITK_INFO << "CHECK PRECONDITIONS ...[DONE]";
}

// Initialize Outputs
void mitk::pa::SpectralUnmixingFilter::InitializeOutputs()
{
  numberOfInputs = m_Dimensions[2];
  numberOfOutputs = GetNumberOfIndexedOutputs();
  for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
  {
    GetOutput(outputIdx)->Initialize(GetInput(0));
  }
}

//Matrix with #chromophores rows and #wavelengths columns
//so Matrix Element (j,i) contains the Absorbtion of chromophore j @ wavelength i
Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::SpectralUnmixingFilter::AddEndmemberMatrix()
{
  numberofchromophores = m_Chromophore.size();
  numberofwavelengths = m_Wavelength.size();

  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix(numberofchromophores, numberofwavelengths);

  //loop over j rows (Chromophores)
  for(unsigned int j =0; j < numberofchromophores; ++j)
  {
    //loop over i columns (Wavelength)
    for (unsigned int i = 0; i < numberofwavelengths; ++i)
    {
      //writes @ Matrix element (i,j) the absorbtion wavelength of the propertycalculator.cpp
      EndmemberMatrix(j,i)= m_PropertyCalculator->GetAbsorptionForWavelength(
      static_cast<mitk::pa::PropertyCalculator::MapType>(m_Chromophore[j]), m_Wavelength[i]);
      /* Test to see what gets written in the Matrix:
      MITK_INFO << "map type: " << static_cast<mitk::pa::PropertyCalculator::MapType>(m_Chromophore[j]);
      MITK_INFO << "wavelength: " << m_Wavelength[i];
      MITK_INFO << "Matrixelement: (" << j << ", " << i << ") Absorbtion: " << EndmemberMatrix(j, i);/**/
      
      if (EndmemberMatrix(j, i) == 0)
      {
        m_Wavelength.clear();
        mitkThrow() << "WAVELENGTH "<< m_Wavelength[i] << "nm NOT SUPPORTED!";
      }
   
    }
  }
  //MITK_INFO << "GENERATING ENMEMBERMATRIX SUCCESSFUL!";
  return EndmemberMatrix;
}

// Perform SU algorithm
Eigen::VectorXd mitk::pa::SpectralUnmixingFilter::SpectralUnmixingAlgorithms(Eigen::VectorXd inputVector)
{
  EndmemberMatrix = AddEndmemberMatrix();
  Eigen::VectorXd resultVector = EndmemberMatrix.householderQr().solve(inputVector);
  return resultVector;
}

/* +++++++++++++++++++++++++++++++++++++++++ OLD CODE: +++++++++++++++++++++++++++++++++++++++++++++++++++
was @ generate data

length = GetOutput(0)->GetDimension(0)*GetOutput(0)->GetDimension(1)*GetOutput(0)->GetDimension(2);

for (int i = 0; i < length; i++)
{
Eigen::VectorXd b(numberOfInputs);
for (unsigned int j = 0; j < numberOfInputs; j++)
{
mitk::Image::Pointer input = GetInput(j);
mitk::ImageReadAccessor readAccess(input, input->GetVolumeData());
b(j) = ((const float*)readAccess.GetData())[i];
}

Eigen::Vector3d x = EndmemberMatrix.householderQr().solve(b);


if (i == 0)
{
MITK_INFO << "for i=0 b(#Inputs)";
MITK_INFO << b;
MITK_INFO << "EndmemberMatrix";
MITK_INFO << EndmemberMatrix;
MITK_INFO << "x";
MITK_INFO << x;
}


for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
{
mitk::Image::Pointer output = GetOutput(outputIdx);
mitk::ImageWriteAccessor writeOutput(output, output->GetVolumeData());
double* outputArray = (double *)writeOutput.GetData();
outputArray[i] = x[outputIdx];
}
}
/**/
