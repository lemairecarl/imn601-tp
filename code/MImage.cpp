#include "MImage.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "gc/GCoptimization.h"
#include <vector>
#include <algorithm>

MImage::MImage(int xs, int ys, int zs)
{
	MXS = 0;
	MYS = 0;
	MZS = 0;
	MImgBuf = NULL;

	if (xs>0 && ys>0 && zs>0)
		MAllocMemory(xs, ys, zs);

	for (int y = 0;y<MYS;y++)
		for (int x = 0;x<MXS;x++)
			MSetColor(0, x, y);
}

MImage::MImage(int xs, int ys, int zs, float color)
{
	MXS = 0;
	MYS = 0;
	MZS = 0;
	MImgBuf = NULL;

	if (xs>0 && ys>0 && zs>0)
		MAllocMemory(xs, ys, zs);

	for (int y = 0;y<MYS;y++)
		for (int x = 0;x<MXS;x++)
			MSetColor(color, x, y);
}

MImage::MImage(void)
{
	MXS = 0;
	MYS = 0;
	MZS = 0;
	MImgBuf=NULL;
}

MImage::MImage(const MImage &copy)
{
 	MXS = 0;
 	MYS = 0;
 	MZS = 0;
 	MImgBuf=NULL;
	if(copy.MIsEmpty()){
		return;
	}

	MAllocMemory(copy.MXS,copy.MYS,copy.MZS);

	for(int y=0;y<MYS;y++)
		for(int x=0;x<MXS;x++)
			MImgBuf[x][y]=copy.MImgBuf[x][y];

}

MImage::~MImage(void)
{
	MFreeMemory();
}

/* =================================================================================
====================================================================================
======================                    I/O                 ======================
====================================================================================
====================================================================================*/

/*
	Allocates a xs x ys x zs image buffer
*/
void MImage::MAllocMemory(int xs,int ys,int zs)
{
	MFreeMemory();
	if(xs<=0||ys<=0||zs<=0){
		printf("Error !! MImage::MAllocMemory\n");
		return;
	}

	MXS = xs;
	MYS = ys;
	MZS = zs;

	MImgBuf = new RGBPixel*[xs];

	for(int i=0;i<xs;i++)
		MImgBuf[i] = new RGBPixel[ys];
}

/*
	Free the image buffer
*/
void MImage::MFreeMemory(void)
{
	if(MImgBuf==NULL || MXS<=0 || MYS<=0 || MZS<=0){
		MXS=MYS=MZS=0;
	}else {
		for(int i=0;i<MXS;i++)
			delete []MImgBuf[i];
		delete []MImgBuf;
		MImgBuf=NULL;
		MXS=MYS=MZS=0;
	}
}

/*
	load a pgm/ppm image
*/
bool MImage::MLoadImage(const char *fileName)
{

	if(fileName==NULL){
		printf("Error!!! MImage::MLoadImage\n");
		return false;
	}
	FILE_FORMAT ff;
	char tmpBuf[500];
	std::ifstream inFile;
	int maxVal,val;
	char valRaw;
	unsigned char color;

	inFile.open (fileName,std::ios::in);
	if (!inFile.is_open()) {
		printf("Error!!! cant open file %s\n",fileName);
		return false;
	}

	inFile.getline(tmpBuf,500);
	switch(tmpBuf[1]){
		case '2':
			ff=PGM_ASCII;
			MZS=1;
		break;
	}

	int nbComm=0;
	inFile.getline(tmpBuf,500);
	while(tmpBuf[0]=='#'){
		nbComm++;
		inFile.getline(tmpBuf,500);
	}
	inFile.close();

	if(ff==PGM_ASCII)
		inFile.open (fileName,std::ios::in);
	else
		inFile.open (fileName,std::ios::in|std::ios::binary);

	inFile.getline(tmpBuf,500);
	while(nbComm>0){
		nbComm--;
		inFile.getline(tmpBuf,500);
	}

	inFile>>MXS;
	inFile>>MYS;
	inFile>>maxVal;

	MAllocMemory(MXS,MYS,MZS);
	switch(ff){

		case PGM_ASCII:
			for(int y=0;y<MYS;y++)
				for(int x=0;x<MXS;x++){
					inFile>>val;
					MImgBuf[x][y].r = (float)val*255.0/maxVal;
				}
		break;

	}

  inFile.close();
 	printf("File %s opened successfully\n",fileName);
 return true;
}

/*
	save a pgm/ppm image
*/
bool MImage::MSaveImage(const char *fileName, FILE_FORMAT ff)
{
	if(!fileName){
		printf("Error!! MImage::MSaveImage\n");
		return false;
	}
	unsigned char val;
	std::ofstream outFile;

	outFile.open (fileName,std::ios::out|std::ios::binary);
	if (!outFile.is_open()) {
		printf("Error!!! cant open file %s\n",fileName);
		return false;
	}
	switch(ff){
		case PGM_ASCII:
			outFile << "P2" << "\n" << MXS << " "  << MYS << "\n"  << "255" << "\n";
			for(int y=0;y<MYS;y++){
				for(int x=0;x<MXS;x++){
					outFile << (unsigned short)(MImgBuf[x][y].r) << " ";
				}
				outFile << "\n";
			}
		break;
	}
 	outFile.close();
	printf("File %s saved successfully\n",fileName);
	return true;
}



/* =================================================================================
====================================================================================
======================           Point Operations             ======================
====================================================================================
====================================================================================*/


/*
	Every pixel with an intensity > 'tvalue' are set to 255.  The other ones are set to 0.
*/
void MImage::MThreshold(float tvalue)
{
	for(int y=0;y<MYS;y++)
		for(int x=0;x<MXS;x++){

			if(MZS>1){

				if((MImgBuf[x][y].r + MImgBuf[x][y].g+ MImgBuf[x][y].b)/3.0>tvalue){
					MImgBuf[x][y].r = 255;
					MImgBuf[x][y].g = 255;
					MImgBuf[x][y].b = 255;
				}else{
					MImgBuf[x][y].r = 0;
					MImgBuf[x][y].g = 0;
					MImgBuf[x][y].b = 0;
				}

			}	else {
				if(MImgBuf[x][y].r>tvalue)
					MImgBuf[x][y].r = 255;
				else
					MImgBuf[x][y].r = 0;
			}
		}
}

/*
	Rescale the image between 0 and 255
*/
void MImage::MRescale(void)
{
	float maxR,minR;
	float maxG,minG;
	float maxB,minB;

	maxR = maxG = maxB = -99999;
	minR = minG = minB =  99999;
	int X,Y;
	for(int y=0;y<MYS;y++)
		for(int x=0;x<MXS;x++){

			if(MImgBuf[x][y].r>maxR)
				maxR = MImgBuf[x][y].r;
			if(MImgBuf[x][y].r<minR)
				minR = MImgBuf[x][y].r;

			if(	MZS>1){
				if(MImgBuf[x][y].g>maxG)
					maxG = MImgBuf[x][y].g;
				if(MImgBuf[x][y].b>maxB)
					maxB = MImgBuf[x][y].b;

				if(MImgBuf[x][y].g<minG)
					minG = MImgBuf[x][y].g;
				if(MImgBuf[x][y].b<minB)
					minB = MImgBuf[x][y].b;
			}
		}

	for(int y=0;y<MYS;y++)
		for(int x=0;x<MXS;x++){
			MImgBuf[x][y].r = (MImgBuf[x][y].r-minR)*255.0/(maxR-minR);

			if(MZS>1){
				MImgBuf[x][y].g = (MImgBuf[x][y].g-minG)*255.0/(maxG-minG);
				MImgBuf[x][y].b = (MImgBuf[x][y].b-minB)*255.0/(maxB-minB);
			}
		}
}




/*
	Mean shift filtering
	
	the implementation is inspired of the following paper

	D. Comanicu, P. Meer: "Mean shift: A robust approach toward feature space analysis".
    IEEE Trans. Pattern Anal. Machine Intell., May 2002.

	The resulting filtered image is copied in the current image (this->MImgBuf)
*/
void MImage::MMeanShift(float SpatialBandWidth, float RangeBandWidth, float tolerance)
{

}
/* =================================================================================
====================================================================================
======================            Feature extraction          ======================
====================================================================================
====================================================================================*/



/*
	Segmentation with magic wand algorithm

	(xSeed, ySeed) is where the region starts growing and
	tolerance is the criteria to stop the region from growing.

*/
void MImage::MMagicWand(int xSeed, int ySeed, float tolerance)
{
	MImage Y(MXS, MYS, 1, -1.0f);
	Flood(Y, xSeed, ySeed, tolerance, MImgBuf[xSeed][ySeed]);

    // Output segmentation result in the image
    for (int x = 0; x < MXS; x++)
    {
        for (int y = 0; y < MYS; y++)
        {
            float grayLevel = float(Y.MGetColor(x, y) > 0.0f) * 255.0f; // Convert boolean result to float
            MSetColor(grayLevel, x, y);
        }
    }
}

void MImage::Flood(MImage &yOut, int xSeed, int ySeed, float tolerance, RGBPixel &xRef)
{
	for (int x = xSeed - 1; x <= xSeed + 1; x++)
	{
		for (int y = ySeed - 1; y <= ySeed + 1; y++)
		{
			// Check coord is inside image AND pixel not visited
            if (x < 0 || x >= MXS || y < 0 || y >= MYS || yOut.MGetColor(x, y) > -1.0f)
                continue;

            if (fabsf(MGetColor(x, y) - xRef.r) < tolerance) // Check inside range
            {
                yOut.MSetColor(1.0f, x, y);
                Flood(yOut, x, y, tolerance, xRef);
            }
            else
            {
                yOut.MSetColor(0.0f, x, y);
            }
		}
	}
}

/*
	N-class segmentation.  Each class is a Gaussian function defined by
	a mean, stddev and prior value.

	The resulting label Field is copied in the current image (this->MImgBuf)
*/
void MImage::MOptimalThresholding(float *means, float *stddev, float *apriori, int nbClasses)
{
	if(nbClasses <= 1)
    {
        std::cout << "Error : number of classes should be at least 2!" << std::endl;
        return;
    }
    
    int classRange = floor(255.0f / nbClasses);
    float sqTwoPi = sqrtf(2.0f * M_PI);
    
    for(int y = 0; y < MYSize(); y++)
    {
        for(int x = 0; x < MXSize(); x++)
        {
            int c = 0;
            float maxProb = 0.0f;
            for(int k = 0; k < nbClasses; k++)
            {
                float diff = powf(MGetColor(x, y) - means[k], 2.0f);
                float prob = apriori[k] * exp2f(-diff / (2.0f * powf(stddev[k], 2.0f))) / (sqTwoPi * stddev[k]);
                if(prob > maxProb)
                {
                    maxProb = prob;
                    c = k;
                }
            }
            MSetColor(c * classRange, x, y);
        }
    }
}


/*
	N-class KMeans segmentation

	Resulting values are copied in parameters 'means','stddev', and 'apriori'
	The 'apriori' parameter contains the proportion of each class.

	The resulting label Field is copied in the current image (this->MImgBuf)
*/
void MImage::MKMeansSegmentation(float *means,float *stddev,float *apriori, int nbClasses)
{

}


/*
	N-class Soft KMeans segmentation

	Resulting values are copied in parameters 'means' and 'stddev'.
	The 'apriori' parameter contains the proportion of each class.

	The resulting label Field is copied in the current image (this->MImgBuf)
*/
void MImage::MSoftKMeansSegmentation(float *means,float *stddev,float *apriori,float beta, int nbClasses)
{
}


/*
	N-class Expectation maximization segmentation
	
	init values are in 'means', 'stddev' and 'apriori'

	Resulting values are copied in parameters 'means', 'stddev' and 'apriori'
	The 'apriori' parameter contains the proportion of each class.

	The resulting label Field is copied in the current image (this->MImgBuf)
*/
void MImage::MExpectationMaximization(float *means,float *stddev,float *apriori, int nbClasses)
{
}

/*
	N-class ICM segmentation

	beta : Constant multiplying the apriori function

	The resulting label Field is copied in the current image (this->MImgBuf)

*/
void MImage::MICMSegmentation(float beta, int nbClasses)
{
}

/*
	N-class Simulated annealing segmentation

	beta : Constant multiplying the apriori function
	Tmax : Initial temperature (initial temperature)
	Tmin : Minimal temperature allowed (final temperature)
	coolingRate : rate by which the temperature decreases

	The label Field copied in the current image (this->MImgBuf)

*/
void MImage::MSASegmentation(float beta,float Tmin, float Tmax, float coolingRate, int nbClasses)
{
}


/*
 * Smooth cost function for use with GCoptimizationGridGraph::setSmoothCost(SmoothCostFnExtra, void*)
*/
double SmoothCost(int s1, int s2, int l1, int l2, void *data)
{
    float* floatData = (float*)data;
    float sigma = floatData[0];
    float* pixData = floatData + 1;

    return double(1000.0f * fabsf(l1 - l2) * expf(-0.5 * powf((pixData[s1] - pixData[s2]) / sigma, 2.0f)));
}

/*
	Interactive graph cut segmentation
	
	the implementation is inspired of the following paper

	Y Boykov and M-P Jolly "Interactive Graph Cuts for Optimal Boundary & Region Segmentation of Objects in N-D images". 
	In International Conference on Computer Vision, (ICCV), vol. I, pp. 105-112, 2001.

	The resulting label Field is copied in the current image (this->MImgBuf)
*/
void MImage::MInteractiveGraphCutSegmentation(MImage &mask, float sigma)
{
    GCoptimizationGridGraph gc(MXSize(), MYSize(), 2);
    float lambda(0.001f);

    // Get means for background and foreground
    float foreSum(0.0f), backSum(0.0f);
    int foreCount(0), backCount(0);
    for (int x = 0; x < MXSize(); ++x)
    {
        for (int y = 0; y < MYSize(); ++y)
        {
            if (mask.MGetColor(x, y) > 250.0f)
            {
                // Foreground selection
                foreSum += MGetColor(x, y);
                foreCount++;
            }
            else if (mask.MGetColor(x, y) > 10.0f)
            {
                // Background selection
                backSum += MGetColor(x, y);
                backCount++;
            }
        }
    }
    float foreground = foreSum / foreCount;
    float background = backSum / backCount;

    // Make 1D vector
    int length = MXSize() * MYSize();
    float* data = new float[length + 1];
    data[0] = sigma;
    float* pixelData = data + 1;
    int pInd = 0;
    for (int x = 0; x < MXSize(); ++x)
    {
        for (int y = 0; y < MYSize(); ++y)
        {
            pixelData[pInd++] = MGetColor(x, y);
        }
    }

    // Data cost
    const float highCost = 1e7;
    for (int x = 0; x < MXSize(); ++x)
    {
        for (int y = 0; y < MYSize(); ++y)
        {
            int pixInd = x * MYSize() + y;

            // Foreground: 1, Background: 0
            if (mask.MGetColor(x, y) > 250.0f)
            {
                // Foreground selection
                gc.setDataCost(pixInd, 0, highCost);
                gc.setDataCost(pixInd, 1, 0.0f); // No cost for choosing foreground
            }
            else if (mask.MGetColor(x, y) > 10.0f)
            {
                // Background selection
                gc.setDataCost(pixInd, 0, 0.0f); // No cost for choosing background
                gc.setDataCost(pixInd, 1, highCost);
            }
            else
            {
                // Not selected
                gc.setDataCost(pixInd, 0, lambda * powf(MGetColor(x, y) - background, 2.0f));
                gc.setDataCost(pixInd, 1, lambda * powf(MGetColor(x, y) - foreground, 2.0f));
            }
        }
    }

    // Smooth cost
    gc.setSmoothCost(SmoothCost, data);

    // Run the algorithm
    gc.expansion(6);

    // Get results
    pInd = 0;
    for (int x = 0; x < MXSize(); ++x)
    {
        for (int y = 0; y < MYSize(); ++y)
        {
            MSetColor(gc.whatLabel(pInd++), x, y);
        }
    }

    MRescale();
}


/* =================================================================================
====================================================================================
======================              Operators                 ======================
====================================================================================
====================================================================================*/
void MImage::operator= (const MImage &copy)
{
	if(copy.MIsEmpty()){
		MFreeMemory();
		return;
	}

	if(!MSameSize(copy))
		MAllocMemory(copy.MXS,copy.MYS,copy.MZS);

	for(int y=0;y<MYS;y++)
		for(int x=0;x<MXS;x++)
			MImgBuf[x][y]=copy.MImgBuf[x][y];

}

void MImage::operator= (float val)
{
	if(MIsEmpty()){
		return;
	}

	for(int y=0;y<MYS;y++)
		for(int x=0;x<MXS;x++)
			for(int z=0;z<MZS;z++)
				MSetColor(val,x,y,z);

}

/* =================================================================================
====================================================================================
======================              Ohters                 ======================
====================================================================================
====================================================================================*/


float MImage::MPottsEnergy(const MImage &img, int x, int y, int label) const
{
}

float MImage::MComputeGlobalEnergy(const MImage &X, float *mean, float *stddev, float beta, int nbClasses) const
{
}

