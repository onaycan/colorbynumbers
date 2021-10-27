#include "headers.h"
#include "CImg.h"
#include <vector>
#include <ctime>
#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <dirent.h>
#include <string>

#include <stdio.h>
#include <stdlib.h>

//Mkl libraries
//#include "mkl.h" 
//#include "mkl_lapack.h" 
//#include "mkl_blas.h" 
//#include "mkl_service.h"
//#include "mkl_spblas.h"
//#include "mkl_pardiso.h"
//#include "mkl_dss.h"

using namespace std;
using namespace cimg_library;


class cluster
{
public:
int id;
float rgb[3];
double rgbsum[3];
int numbers;
double prev_grayscale;
double current_grayscale;
};

class pixel
{
public:
int row;
int column; 
float rgb[3];
int pixelcluster;

};



int main(int argc,char *argv[])
{
int noofclusters=25;
cluster* clusters;
clusters=new cluster[noofclusters];


ofstream histogram("./histogram.txt");

string master_pic;
int i=0;
DIR *dir;

struct dirent *pic_under_dir;

dir = opendir("master_pic");
if (dir != NULL){
   while (pic_under_dir = readdir(dir)){
      //cout<<pic_under_dir->d_name<<endl;
      if(i>=2)
      master_pic="master_pic/"+string(pic_under_dir->d_name);
      i++;
      }
}

cout<<"Following master Picture is taken: "<<endl;
cout<<master_pic<<endl;

CImg < float > master_img(master_pic.c_str());
int mw = master_img.width();
int mh = master_img.height();
int md = master_img.depth();
//cout<<mw<<endl;
//cout<<mh<<endl;

const CImg< float > hist = master_img.get_histogram(256);
//hist.display_graph(0,3);
for(int i=0;i<256;i++)
histogram<<hist[i]<<endl;

pixel* pixels;
pixels=new pixel[mw*mh];
float mins[3];
float maxs[3];
for(int i=0;i<3;i++)
{
mins[i]=256;
maxs[i]=0;
}



for(int r=0;r<mh;r++)
for(int c=0;c<mw;c++)
{
int pixelcounter=r*mw+c;
pixels[pixelcounter].row=r;
pixels[pixelcounter].column=c;
for(int i=0;i<3;i++)
{
pixels[pixelcounter].rgb[i]=master_img(c,r,0,i);
if(master_img(c,r,0,i)<mins[i])
mins[i]=master_img(c,r,0,i);
if(master_img(c,r,0,i)>maxs[i])
maxs[i]=master_img(c,r,0,i);
}

}


for(int i=0;i<noofclusters;i++)
{
clusters[i].id=i;
clusters[i].prev_grayscale=0.0;
clusters[i].current_grayscale=0.0;

for(int j=0;j<3;j++)
{
clusters[i].rgb[j]=mins[j]+(maxs[j]-mins[j])/double(noofclusters)*double(i);
clusters[i].rgbsum[j]=clusters[i].rgb[j];
clusters[i].numbers=1;
}
clusters[i].prev_grayscale = round(0.299*((double)clusters[i].rgb[0]) + 0.587*((double)clusters[i].rgb[1]) + 0.114*((double)clusters[i].rgb[2]));

}

//for(int k=0;k<5;k++)
double divergence=1.0;
int initialflag=0;
while(divergence>0.5)
{
divergence=0.0;
initialflag++;
for(int r=0;r<mh;r++)
for(int c=0;c<mw;c++)
{
//cout<<r<<endl;
//cout<<c<<endl;
int pixelcounter=r*mw+c;
double smallest=0.0;
for(int i=0;i<3;i++)
smallest=smallest+(pixels[pixelcounter].rgb[i]-clusters[0].rgb[i])*(pixels[pixelcounter].rgb[i]-clusters[0].rgb[i]);
smallest=sqrt(smallest);
int minloc=0;

//do this only in the beginning, namely the first loop
if(initialflag==1)
{
clusters[minloc].numbers=clusters[minloc].numbers+1;
for(int i=0;i<3;i++)
{
clusters[minloc].rgbsum[i]=clusters[minloc].rgbsum[i]+pixels[pixelcounter].rgb[i];
clusters[minloc].rgb[i]=clusters[minloc].rgbsum[i]/clusters[minloc].numbers;
}
pixels[pixelcounter].pixelcluster=minloc;
}

for(int cl=0;cl<noofclusters;cl++)
{
double current=0.0;
//clusters[cl].prev_grayscale = round(0.299*(clusters[cl].rgb[0]) + 0.587*(clusters[cl].rgb[1]) + 0.114*(clusters[cl].rgb[2]));
for(int i=0;i<3;i++)
current=current+(pixels[pixelcounter].rgb[i]-clusters[cl].rgb[i])*(pixels[pixelcounter].rgb[i]-clusters[cl].rgb[i]);
current=sqrt(current);
if(current<smallest)
{
smallest=current;
minloc=cl;
}
}


if(minloc!=pixels[pixelcounter].pixelcluster)
{
clusters[pixels[pixelcounter].pixelcluster].numbers=clusters[pixels[pixelcounter].pixelcluster].numbers-1;
for(int i=0;i<3;i++)
{
clusters[pixels[pixelcounter].pixelcluster].rgbsum[i]=clusters[pixels[pixelcounter].pixelcluster].rgbsum[i]-pixels[pixelcounter].rgb[i];
clusters[pixels[pixelcounter].pixelcluster].rgb[i]=clusters[pixels[pixelcounter].pixelcluster].rgbsum[i]/clusters[pixels[pixelcounter].pixelcluster].numbers;
}

clusters[minloc].numbers=clusters[minloc].numbers+1;
for(int i=0;i<3;i++)
{
clusters[minloc].rgbsum[i]=clusters[minloc].rgbsum[i]+pixels[pixelcounter].rgb[i];
clusters[minloc].rgb[i]=clusters[minloc].rgbsum[i]/clusters[minloc].numbers;
}

pixels[pixelcounter].pixelcluster=minloc;
}



}
for(int cl=0;cl<noofclusters;cl++)
{
clusters[cl].current_grayscale = round(0.299*(clusters[cl].rgb[0]) + 0.587*(clusters[cl].rgb[1]) + 0.114*(clusters[cl].rgb[2]));
double current_divergence=(clusters[cl].current_grayscale-clusters[cl].prev_grayscale)/256.0*100.0;
if(current_divergence>divergence)
divergence=current_divergence;
clusters[cl].prev_grayscale = round(0.299*(clusters[cl].rgb[0]) + 0.587*(clusters[cl].rgb[1]) + 0.114*(clusters[cl].rgb[2]));
}

cout<<"percentual divergence of clustering: "<<divergence<<endl;
}




//GENERATING THE GRAYSCALES, SOBEL ALGORITHS, AND SO ON 


CImg<float> total_gray(mw,mh,md,1);
CImg<float> total_clustered(mw,mh,md,3);
CImg<float> desired_clustered(mw,mh,md,3);

vector< CImg<float> > cluster_pics;
cluster_pics.resize(noofclusters); 
vector< CImg<float> > gray_pics;
gray_pics.resize(noofclusters); 



for(int cl=0;cl<noofclusters;cl++)
{
//getting the current cluster region
CImg<float> current_cluster(mw,mh,md,3);
CImg<float> current_sobel(mw,mh,md,3);
CImg<float> current_gray(mw,mh,md,1);

for(int r=0;r<mh;r++)
for(int c=0;c<mw;c++)
{
int pixelcounter=r*mw+c;
if(clusters[pixels[pixelcounter].pixelcluster].id==cl)
for(int i=0;i<3;i++)
current_cluster(c,r,0,i)=int(clusters[pixels[pixelcounter].pixelcluster].rgb[i]);
else
for(int i=0;i<3;i++)
current_cluster(c,r,0,i)=255;
}
/*
//convolution and mask for edge sharpening 
for(int k=0;k<1;k++)
{
const CImg<float> mask(3,3,1,1, 0.125,0.125,0.125, 0.125,0,0.125,0.125,0.125,0.125),
img = current_cluster.get_convolve(mask);
current_cluster=img;
}
*/
//sobel convolution of the current cluster region
const CImg<float> maskX(3,3,1,1, 1,0,-1, 2,0,-2,1,0,-1), maskY = maskX.get_transpose(),
imgX = current_cluster.get_convolve(maskX), imgY = current_cluster.get_convolve(maskY);

cimg_forXYC(current_cluster, x, y, v) {
     current_sobel(x, y, 0, v) = sqrt( imgX(x,y,0,v)*imgX(x,y,0,v) + imgY(x,y,0,v)*imgY(x,y,0,v) );
}

//gray scaling of the current cluster region
for(int r=0;r<mh;r++)
for(int c=0;c<mw;c++)
{
for(int i=0;i<3;i++)
current_sobel(c,r,0,i)=255-current_sobel(c,r,0,i);

double gr = round(0.299*((double)current_sobel(c,r,0,0)) + 0.587*((double)current_sobel(c,r,0,1)) + 0.114*((double)current_sobel(c,r,0,2)));
current_gray(c,r,0,0) = gr;

}

cluster_pics[cl]=current_cluster;
gray_pics[cl]=current_gray;
}

for(int cl=0;cl<noofclusters;cl++)
{
cimg_forXYC(total_gray, x, y, v) {
     total_gray(x, y, 0, v) = total_gray(x, y, 0, v) + gray_pics[cl](x, y, 0, v);
}
}

cimg_forXYC(total_gray, x, y, v) {
     total_gray(x, y, 0, v) = total_gray(x, y, 0, v)/noofclusters;
}


int total_numbers=0;
string extension=master_pic.substr(master_pic.find('.'),100);

for(int cl=0;cl<noofclusters;cl++)
{
char c[10];
snprintf(c, 10, "%d", cl);
string picname=master_pic.substr(0,master_pic.find(extension));
//picname.insert(picname.begin(),"./outputs/");
picname.append("_RGB_");
char red[10];
char gr[10];
char bl[10];
snprintf(red, 10, "%d", (int)clusters[cl].rgb[0]);
snprintf(gr, 10, "%d", (int)clusters[cl].rgb[1]);
snprintf(bl, 10, "%d", (int)clusters[cl].rgb[2]);
picname.append(red);
picname.append("&");
picname.append(gr);
picname.append("&");
picname.append(bl);
picname.append("_cluster_");
picname.append(c);
picname.append(extension);
cout<<picname<<'\t'<<clusters[cl].numbers<<endl;
total_numbers=total_numbers+clusters[cl].numbers;
for(int r=0;r<mh;r++)
for(int c=0;c<mw;c++)
{
int pixelcounter=r*mw+c;
if(clusters[pixels[pixelcounter].pixelcluster].id!=cl)
for(int i=0;i<3;i++)
cluster_pics[cl](c,r,0,i)=total_gray(c,r,0,0);
else
for(int i=0;i<3;i++)
total_clustered(c,r,0,i)=clusters[cl].rgb[i];

}



cluster_pics[cl].save(picname.c_str());

}
for(int r=0;r<mh;r++)
for(int c=0;c<mw;c++)
for(int i=0;i<3;i++)
desired_clustered(c,r,0,i)=total_gray(c,r,0,0);


int desired_clusters[12]={18,19,24,27,4,28,6,12,17,9,21,14};

for(int clc =0;clc<12;clc++)
{
int current_cl=desired_clusters[clc];
for(int r=0;r<mh;r++)
for(int c=0;c<mw;c++)
{
int pixelcounter=r*mw+c;
if(clusters[pixels[pixelcounter].pixelcluster].id==current_cl)
for(int i=0;i<3;i++)
desired_clustered(c,r,0,i)=clusters[current_cl].rgb[i];

}
}


string picname=master_pic.substr(0,master_pic.find("."));
//picname.insert(picname.begin(),'./outputs/');
picname.append("_grayscale");
picname.append(extension);
total_gray.save(picname.c_str());

picname=master_pic.substr(0,master_pic.find("."));
//picname.insert(picname.begin(),'./outputs/');
picname.append("_normal");
picname.append(extension);
total_clustered.save(picname.c_str());

picname=master_pic.substr(0,master_pic.find("."));
//picname.insert(picname.begin(),'./outputs/');
picname.append("_desired_clustered");
picname.append(extension);
desired_clustered.save(picname.c_str());



cout<<"number of pixels: "<<mw*mh<<endl;
cout<<"clustered pixels: "<<total_numbers<<endl;


int numberof_graynodes=0;
ofstream outputfile("input_for_mesh.txt");
cimg_forXYC(total_gray,x, y, v) {
     int grayscale=(int)total_gray(x,y,0,v);
	 if (grayscale<225)
		numberof_graynodes++;
		//outputfile<<x<<'\t'<<y<<'\t'<<0<<endl;
	 //master_img(x, y, 0, v);
}
outputfile<<"2"<<endl;
outputfile<<numberof_graynodes<<endl;
cimg_forXYC(total_gray,x, y, v) {
     int grayscale=(int)total_gray(x,y,0,v);
	 if (grayscale<225)
		outputfile<<x<<'\t'<<y<<endl;
	 //master_img(x, y, 0, v);
}






//master_img.save("yarro.jpg");


//const CImg< float > img2 = master_img.get_histogram(20);
//hist2.display_graph(0,3);




/*
ofstream outfile("./sources/finish_flag.log");
clock_t t;
t = clock();
t = clock() - t;



//variables
string master_pic;
vector<string> slave_pics;
int numberofslave_pics;
int division;
int mweight;
int sweight;
int enlargement;
string result;

ifstream infile("./sources/config_final.log");
string info; 
getline(infile,info);
infile>>master_pic;
getline(infile,info);
getline(infile,info);
infile>>numberofslave_pics;
getline(infile,info);
for(int i=0;i<numberofslave_pics;i++)
{
string current_pic;
infile>>current_pic;
slave_pics.push_back(current_pic);
}
getline(infile,info);
getline(infile,info);
infile>>division;
getline(infile,info);
getline(infile,info);
infile>>enlargement;
getline(infile,info);
getline(infile,info);
infile>>mweight;
getline(infile,info);
getline(infile,info);
infile>>sweight;
getline(infile,info);
getline(infile,info);
infile>>result;






int div=100/division;
CImg < unsigned char > master_img(master_pic.c_str());
int mw = master_img.width();
int mh = master_img.height();
double master_x_interval=double(mw)/double(division)*enlargement;
double master_y_interval=double(mh)/double(division)*enlargement;
master_img.resize(int(master_x_interval)*division,int(master_y_interval)*division,-100,-100,5); 
//master_img.blur_anisotropic(10);
mw = master_img.width();
mh = master_img.height();
CImg < unsigned char > visu(mw,mh,1,3);
//cout<<"Normalized pix dimensions of the master image:"<<endl;
//cout<<mw<<endl;
//cout<<mh<<endl;

visu.fill(0).draw_image(0,0,0,0,master_img,1);;

int counter=0; 
int numberofslaves=slave_pics.size();


//CImg < unsigned char > slave_img(slave_pics[0].c_str());
//slave_img.resize(int(master_x_interval),int(master_y_interval),-100,-100,3);
//visu.draw_image(0,0,0,0,slave_img);

int prev_master_x_loc=0;
int cur_master_x_loc=0;

for(int rcount=0;rcount<division;rcount++)
{
outfile<<double(rcount)/double(division)*100<<endl;
int ccount=0;
//for(int ccount=0;ccount<division+1;ccount++)
prev_master_x_loc=0;
while(prev_master_x_loc<mw)
{

cout<<slave_pics[rand()%numberofslaves]<<endl;
CImg < unsigned char > slave_img(slave_pics[rand()%numberofslaves].c_str());
int sw = slave_img.width();
int sh = slave_img.height();	






double d_slave_x_scale=double(sw)*master_y_interval/double(sh);

slave_img.resize(int(d_slave_x_scale),int(master_y_interval),-100,-100);

counter++;

sw = slave_img.width();
sh = int(master_y_interval);


prev_master_x_loc=cur_master_x_loc;
cur_master_x_loc=cur_master_x_loc+sw;
if(ccount==0)
{
cur_master_x_loc=sw;
prev_master_x_loc=0;
}
ccount++;

for (int r = rcount*int(master_y_interval); r < (rcount+1)*int(master_y_interval); r++)
for (int c = prev_master_x_loc; c < cur_master_x_loc; c++)
{
//int r_cur=r-rcount*int(master_x_interval);
//int c_cur=c-ccount*int(master_y_interval);

int x_cur=(c-int(prev_master_x_loc));
int y_cur=r-rcount*int(master_y_interval);

for(int i=0;i<3;i++)
slave_img(x_cur,y_cur,0,i)=(sweight*slave_img(x_cur,y_cur,0,i)+mweight*master_img(c%mw,r%mh,0,i))/(mweight+sweight);

}

visu.draw_image(prev_master_x_loc,rcount*int(master_y_interval),0,0,slave_img);
}
}

//visu.display();
visu.save(result.c_str());


outfile<<"100";
//cout<<"Computation time in Seconds: "<<((float)t)/CLOCKS_PER_SEC<<endl;
*/
return 0;
};
