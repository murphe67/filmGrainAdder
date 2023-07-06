
#include <fstream>
#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdlib.h>

int arg2int(const char* arg);
std::fstream tryOpenFile(const char* fileName);

int main( int argc, char** argv )
{
    std::fstream fin;
    std::fstream fout;
    int goalSigma;
    int width;
    int height;
    int numFrames;

    try{
        if(argc<7) {
            throw std::invalid_argument("Not enough input arguments.\nUsage: ./arFilter [input file] [output file] [sigma] [width] [height] [numFrames]");
        }
        fin = tryOpenFile(argv[1]);
        fout.open(argv[2]);
        goalSigma = arg2int(argv[3]);
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

    cv::Mat_<double> kernel(3, 3);
    //old school
    // kernel <<  0.0232376856854350,  0.0224667770886383, 0.00126609507256231,
    //            0.0253961698437219, -0.00569499178547407, 0.159828992626180,
    //            -0.0126038382871265, 0.254263306400884,  1;

    //grain off youtube
    kernel <<   0.0412877726147980, -0.0160599827082709, -0.142365961087186,
                0.0134909027281112, -0.260534163652160,  0.698540136126559,
                -0.131609214173857, 0.556044445343399;

    cv::Mat grain(height, width, CV_64F);
    cv::Mat out;
    for(int i = 0; i<numFrames; i++){
        fin.read(yFrame, width * height);

        cv::Mat mtx(height, width, CV_8U, yFrame);
        // cv::randn(grain, 0, sqrt(61.65));
        cv::randn(grain, 0, sqrt(0.41));
        for(int y = 2; y<height;++y){
            for(int x = 2; x<width;++x){
                grain.at<double>(y, x) =  
                (grain.at<double>(y-2, x-2) * kernel(0, 0)) +
                (grain.at<double>(y-2, x-1) * kernel(0, 1)) +
                (grain.at<double>(y-2, x  ) * kernel(0,  2)) +
                (grain.at<double>(y-1, x-2) * kernel(1, 0)) +
                (grain.at<double>(y-1, x-1) * kernel(1, 1)) +
                (grain.at<double>(y-1, x  ) * kernel(1, 2)) +
                (grain.at<double>(y  , x-2) * kernel(2, 0)) +
                (grain.at<double>(y  , x-1) * kernel(2, 1)) +
                grain.at<double>(y, x);
            }
        }

        cv::Scalar mean, stddev;

        cv::meanStdDev(grain, mean, stddev);
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

    delete yFrame;
    delete uvFrame;

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

std::fstream tryOpenFile(const char* fileName){
    std::fstream file(fileName);
    if(file.fail()){
        throw std::invalid_argument("Invalid Argument: couldn't open file: " + std::string(fileName));
    }
    return file;
}