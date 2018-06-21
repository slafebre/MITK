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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGSO2_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGSO2_H

#include "mitkImageToImageFilter.h"
#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"


namespace mitk {
  namespace pa {
    /**
    * \brief derives out of two identical sized MITK images the oxygen saturation and return one MITK image as result. Furthermore
    * it is possible to set settings that the result shows just SO2 values above a threshold, or above a input value for Hb, HbO2 to
    * get just a oxygen saturation image of interessting structures.
    *
    * Input:
    * The input has to be two 3D MITK images. The order of the inputs matter! The first input has to be the Hb image the second input
    * has to be the HbO2 image. The settings are integer values. The SO2 threshold therefore is percentage value.
    *
    * Output:
    * The output will be one MITK image. Where one can see the oxygen saturation of all pixels above the set threholds. If a pixel is
    * below a threhold or NAN then the value will be set to zero.
    */
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingSO2 : public mitk::ImageToImageFilter
    {
    public:
      mitkClassMacro(SpectralUnmixingSO2, mitk::ImageToImageFilter);
      itkFactorylessNewMacro(Self);

      /**
      * \brief AddSO2Settings takes integers and writes them at the end of the m_SO2Settings vector.
      * @param value has to be a integer
      */
      virtual void AddSO2Settings(float value);

      /*
      * \brief Verbose gives more information to the console. Default value is false.
      * @param m_Verbose is the boolian to activate the MITK_INFO logged to the console
      */
      virtual void Verbose(bool verbose);

    protected:
      /**
      * \brief Constructor sets number of input images to two and number of output images to one, respectively.
      */
      SpectralUnmixingSO2();
      virtual ~SpectralUnmixingSO2();

      std::vector<int> m_SO2Settings;
      bool m_Verbose = false;

    private:
      /*
      * \brief Inherit from the "ImageToImageFilter" Superclass. Herain it calls InitializeOutputs and the CheckPreConditions
      * methods and enables pixelwise access to the inputs to calculate the oxygen saturation via the "calculate SO2" method.
      */
      virtual void GenerateData() override;

      /*
      * \brief Initialized output images with the same size like the input image. The pixel type is set to float.
      */
      virtual void InitializeOutputs();

      /*
      * \brief Checks if the dimensions of the input images are equal.
      * @throws if tey are not
      */
      virtual void CheckPreConditions(mitk::Image::Pointer inputHbO2, mitk::Image::Pointer inputHb);

      /*
      * \brief calculates HbO2 / (Hb + HbO2) and afterwards checks if the result and inputs are above threshold values of the
      * settings. If they are not the method returns zero otherwise it returns the calculated result.
      * @param pixelHb is the pixel value of the Hb input.
      * @param pixelHb is the pixel value of the Hb input.
      * @warn if the HbO2 value is NAN (in patricular if Hb == -HbO2), but result will be set to zero
      */
      float calculateSO2(float pixelHb, float pixelHbO2);
    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGSO2_
