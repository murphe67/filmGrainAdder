
#include <fstream>
#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdlib.h>

int arg2int(const char* arg);
float arg2float(const char* arg);
std::fstream tryOpenFile(const char* fileName);

int main( int argc, char** argv )
{
    std::fstream fin;
    std::ofstream fout;
    int goalSigma;
    int width;
    int height;
    int numFrames;

    try{
        if(argc<7) {
            throw std::invalid_argument("Not enough input arguments.\nUsage: ./filmGrainAdder [input file] [output file] [sigma] [width] [height] [numFrames]");
        }
        fin = tryOpenFile(argv[1]);
        fout.open(argv[2]);
        goalSigma = arg2float(argv[3]);
        width = arg2int(argv[4]);
        height = arg2int(argv[5]);
        numFrames = arg2int(argv[6]);
    } catch(const std::invalid_argument &e){
        std::cout << e.what() << std::endl;
        return 0;
    }

    std::cout << "Adding film grain to " << std::string(argv[1]) << std::endl;


    char* yFrame = new char[width * height];
    char* uvFrame = new char[width * height / 4];

    int lag = 2;
    cv::Mat_<double> kernel(((lag+1)*(lag+1)) + (lag*lag) -1, 1);

    kernel <<   0.0408973753375396,
                -0.0249732630696668,
                -0.0469093590367031,
                -0.00183072425932764,
                0.00884455096177940,
                0.346663128842722,
                -0.0625607468970479,
                0.325104219351192,
                -0.0441798187985132,
                0.0990816397226081,
                0.0200728890185384,
                0.00289972718613776;

    cv::Mat grain(height, width, CV_64F);
    cv::Mat out;
    for(int i = 0; i<numFrames; i++){
        fin.read(yFrame, width * height);

        cv::Mat mtx(height, width, CV_8U, yFrame);
        cv::randn(grain, 0, sqrt(0.38));

        for(int x = lag + 1; x<width-(lag+1);++x){
            for(int y = lag + 1; y<height;++y){
                double pixel = grain.at<double>(y, x);
                int j = 0;

                for (int xLag = -lag; xLag<=-1;xLag++){
                    for(int yLag = -lag; yLag<=0;yLag++){
                        pixel += (grain.at<double>(y+yLag, x+xLag) * kernel(j, 0));
                        j++;
                    }
                }
                
                for (int xLag = 0; xLag<=lag;xLag++){
                    for(int yLag = -lag; yLag<=-1;yLag++){
                        pixel += (grain.at<double>(y+yLag, x+xLag) * kernel(j, 0));
                        j++;
                    }
                }
                grain.at<double>(y, x) = pixel;
            }
        }

        double minVal; 
        double maxVal; 
        cv::Point minLoc; 
        cv::Point maxLoc;

        cv::minMaxLoc(grain, &minVal, &maxVal, &minLoc, &maxLoc);

        double step  = 2.0 / 5.0;
        for(int x = 0; x<width;++x){
            for(int y = 0; y<height;++y){
                int numSteps = grain.at<double>(y,x) / step;
                grain.at<double>(y,x) = (double)numSteps * step;
            }
        }

        cv::Scalar mean, stddev;

        cv::meanStdDev(grain, mean, stddev);
        grain = grain - mean;
        grain = (grain / stddev) * goalSigma;


        mtx.convertTo(out, CV_64F);
        out = out + grain;
        out.convertTo(mtx, CV_8U);

        char* y = reinterpret_cast<char*>(mtx.data);

        fout.write(y, 1280 * 720);
        
        fin.read(uvFrame, 1280 * 720 / 4);
        fout.write(uvFrame, 1280 * 720 / 4);

        fin.read(uvFrame, 1280 * 720 / 4);
        fout.write(uvFrame, 1280 * 720 / 4);
    }

    delete[] yFrame;
    delete[] uvFrame;

    fin.close();
    fout.close();

    return 0;
}


int arg2int(const char* arg){
    std::istringstream ss(arg);
    int x;
    if (!(ss >> x)) {
        throw std::invalid_argument("Invalid number: " + std::string(arg));
    } else if (!ss.eof()) {
        throw std::invalid_argument("Trailing characters after number: " + std::string(arg));
    }
    return x;
}

float arg2float(const char* arg){
    std::istringstream ss(arg);
    float x;
    if (!(ss >> x)) {
        throw std::invalid_argument("Invalid number: " + std::string(arg));
    } else if (!ss.eof()) {
        throw std::invalid_argument("Trailing characters after number: " + std::string(arg));
    }
    return x;
}

std::fstream tryOpenFile(const char* fileName){
    std::fstream file(fileName);
    if(file.fail()){
        throw std::invalid_argument("Invalid Argument: couldn't open file: " + std::string(fileName));
    }
    return file;
}