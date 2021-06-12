// Project: B*-tree based placement/floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsors: NSC, Taiwan; Arcadia, Inc.; UMC.
// Version 1.0
// Date:    7/19/2000

//---------------------------------------------------------------------------
#include <iostream>             
#include <cstring>
#include "btree.h"
#include "qbtree.h"
#include "sa.h"
//---------------------------------------------------------------------------

int main(int argc,char **argv)
{
   char filename[80],outfile[80]="",outresult[80]="";
   int times=30, local=7;
   float init_temp=0.9, term_temp=0.1;
   float alpha=1;
   srand(time(0));
  
   if(argc<=1){
     printf("Usage: btree <filename> [times=%d] [hill_climb_stage=%d]\n",
           times, local);
     printf("        [avg_ratio=%.1f] [cost_ratio=%f]\n",avg_ratio,alpha);
     printf("        [lamda=%.2f] [term-temp=%.2f]\n",lamda,term_temp);
     printf("        [output]\n");
     return 0;
   }else{
     int argi=1;
     if(argi < argc) strcpy(filename, argv[argi++]);
     if(argi < argc) times=atoi(argv[argi++]);
     if(argi < argc) local=atoi(argv[argi++]);
     if(argi < argc) avg_ratio=atof(argv[argi++]);
     if(argi < argc) alpha=atof(argv[argi++]);
     if(argi < argc) lamda=atof(argv[argi++]);
     if(argi < argc) term_temp=atof(argv[argi++]);
     if(argi < argc) strcpy(outfile, argv[argi++]);
   }

   try{
    QBtree qbt;
    double time = seconds();
    qbt.init(alpha,filename,times,local,term_temp);
    
    double last_time = qbt.SA_Floorplan(times, local, term_temp);
    //qbt.show_module();
    qbt.getCost();
    { // log performance and quality
       if(strlen(outfile)==0){
        strcpy(outfile,filename);
        strcat(outfile,".res");
      }

       last_time = last_time - time;
       printf("CPU time       = %.2f\n",seconds()-time);
       printf("Last CPU time  = %.2f\n",last_time);

       // Appending .res file
       FILE *fs= fopen(outfile,"a+");
       fprintf(fs,"--- constraints --- \n");
       if(!qbt.constraints.max_sep.empty())
       {
         fprintf(fs,"Maximum Seperation constraints : %d \n",qbt.constraints.max_sep.size());
       }
       if(!qbt.constraints.min_sep.empty())
       {
         fprintf(fs,"Minimum Seperation constraints : %d \n",qbt.constraints.min_sep.size());
       }
       if(!qbt.constraints.boundary.empty())
       {
         fprintf(fs,"Boundary constraints : %d \n",qbt.constraints.boundary.size());
       }
       if(!qbt.constraints.fixed_boundary.empty())
       {
         fprintf(fs,"Fixed boundary constraints : %d \n",qbt.constraints.fixed_boundary.size());
       }
       if(!qbt.constraints.clto_boundary.empty())
       {
         fprintf(fs,"Close to boundary constraints : %d \n",qbt.constraints.clto_boundary.size());
       }
       if (!qbt.constraints.proximity.empty())
       {
           fprintf(fs, "Proximity constraints : %d \n", qbt.constraints.proximity.size());
       }
       if (!qbt.constraints.range.empty())
       {
           fprintf(fs, "Range constraints : %d \n", qbt.constraints.range.size());
       }
       if (!qbt.constraints.variant.empty())
       {
           fprintf(fs, "Variant constraints : %d \n", qbt.constraints.variant.size());
       }
       fprintf(fs,"CPU= %.2f, Cost= %.6f, Area= %.0f, Wire= %.0f, Dead=%.4f ",
               last_time, float(qbt.cost),  float(qbt.Area*1e-6),
                float(qbt.WireLength*1e-3), float((qbt.Area - qbt.TotalArea)*1e-6));
       //fprintf(fs," :%d \n", times);
       fprintf(fs,"\n");
       fclose(fs);

       //Creating matlab plot
      strcpy(outresult,filename);
      strcat(outresult,"_output.m");
      qbt.outPutResult(outresult);

      //Display QBtree
      qbt.showQBTree();
    }
  
   }catch(...){}
   return 1;
}
