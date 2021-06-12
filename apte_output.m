%region 
x1=0; 
x2=10858; 
y1=0; 
y2=10858; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','b','LineWidth',1);
%ppm1 
x1=0; 
x2=3186; 
y1=0; 
y2=1832; 
rect = rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.1 .5 .5],'EdgeColor','b','LineWidth',1);
text(x1+5,y2-((y2-y1)/4),'ppm'); 
%module4 
x1=40; 
x2=3186; 
y1=1832; 
y2=3658; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'cc_14'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module6 
x1=0; 
x2=3186; 
y1=3658; 
y2=5490; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'cc_22'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module7 
x1=3186; 
x2=6372; 
y1=0; 
y2=1832; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'cc_23'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module2 
x1=40; 
x2=3186; 
y1=5490; 
y2=7316; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.800000 0.300000 0.100000],'EdgeColor','b','LineWidth',1);
str = 'cc_12'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module1 
x1=3186; 
x2=6332; 
y1=5496; 
y2=7322; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.800000 0.300000 0.100000],'EdgeColor','b','LineWidth',1);
str = 'cc_11'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%region 
x1=759; 
x2=8759; 
y1=2409; 
y2=10409; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor','none','EdgeColor',[0.800000 0.300000 0.100000],'LineWidth',1,'LineStyle','--');
%module3 
x1=3186; 
x2=5012; 
y1=1832; 
y2=4978; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.100000 0.600000 0.400000],'EdgeColor','b','LineWidth',1);
str = 'cc_13'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module5 
x1=5012; 
x2=8198; 
y1=3664; 
y2=5496; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.100000 0.600000 0.400000],'EdgeColor','b','LineWidth',1);
str = 'cc_21'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%region 
x1=1605; 
x2=11605; 
y1=-420; 
y2=9580; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor','none','EdgeColor',[0.100000 0.600000 0.400000],'LineWidth',1,'LineStyle','--');
%module9 
x1=6472; 
x2=7298; 
y1=1546; 
y2=1832; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.400000 0.900000 0.700000],'EdgeColor','b','LineWidth',1);
str = 'clk'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module8 
x1=5012; 
x2=8198; 
y1=1832; 
y2=3664; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.400000 0.900000 0.700000],'EdgeColor','b','LineWidth',1);
str = 'cc_24'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%region 
x1=4605; 
x2=8605; 
y1=748; 
y2=4748; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor','none','EdgeColor',[0.400000 0.900000 0.700000],'LineWidth',1,'LineStyle','--');
%region 
x1=0; 
x2=10858; 
y1=1832; 
y2=1833; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','r','LineWidth',1);
%region 
x1=3186; 
x2=3187; 
y1=0; 
y2=10858; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','r','LineWidth',1);
