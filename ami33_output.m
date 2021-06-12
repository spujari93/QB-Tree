%region 
x1=0; 
x2=1797; 
y1=0; 
y2=1797; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','b','LineWidth',1);
%ppm1 
x1=0; 
x2=560; 
y1=0; 
y2=497; 
rect = rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.1 .5 .5],'EdgeColor','b','LineWidth',1);
text(x1+5,y2-((y2-y1)/4),'ppm'); 
%module1 
x1=735; 
x2=1071; 
y1=497; 
y2=630; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk1'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module2 
x1=182; 
x2=560; 
y1=1043; 
y2=1162; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk10a'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module3 
x1=560; 
x2=721; 
y1=896; 
y2=1036; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk10b'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module5 
x1=560; 
x2=735; 
y1=497; 
y2=616; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk11'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module6 
x1=420; 
x2=560; 
y1=497; 
y2=903; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk12'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module8 
x1=224; 
x2=420; 
y1=497; 
y2=616; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk14a'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module9 
x1=560; 
x2=854; 
y1=1036; 
y2=1155; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk14b'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module10 
x1=1196; 
x2=1357; 
y1=378; 
y2=497; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk14c'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module11 
x1=63; 
x2=182; 
y1=497; 
y2=763; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk15a'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module12 
x1=1120; 
x2=1239; 
y1=616; 
y2=952; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk15b'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module13 
x1=721; 
x2=840; 
y1=896; 
y2=1022; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk16'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module14 
x1=189; 
x2=560; 
y1=1162; 
y2=1344; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk17a'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module15 
x1=1120; 
x2=1302; 
y1=952; 
y2=1155; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk17b'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module16 
x1=854; 
x2=1036; 
y1=763; 
y2=966; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk18'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module17 
x1=476; 
x2=560; 
y1=903; 
y2=1022; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk19'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module18 
x1=560; 
x2=854; 
y1=763; 
y2=896; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk2'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module19 
x1=706; 
x2=1056; 
y1=305; 
y2=487; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk20'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module20 
x1=1120; 
x2=1435; 
y1=1155; 
y2=1295; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk21'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module21 
x1=1372; 
x2=1505; 
y1=749; 
y2=1064; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk3'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module22 
x1=560; 
x2=1120; 
y1=630; 
y2=763; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk4'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module23 
x1=1239; 
x2=1372; 
y1=749; 
y2=889; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk5a'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module24 
x1=1239; 
x2=1414; 
y1=616; 
y2=749; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk5b'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module25 
x1=1196; 
x2=1329; 
y1=147; 
y2=378; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk5c'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module26 
x1=854; 
x2=987; 
y1=1050; 
y2=1365; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk6'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module27 
x1=238; 
x2=420; 
y1=616; 
y2=714; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk7'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module28 
x1=210; 
x2=420; 
y1=714; 
y2=924; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk8a'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module29 
x1=570; 
x2=696; 
y1=109; 
y2=487; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk8b'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module30 
x1=294; 
x2=476; 
y1=924; 
y2=1043; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk9a'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module31 
x1=1329; 
x2=1448; 
y1=259; 
y2=378; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk9b'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module32 
x1=1071; 
x2=1428; 
y1=497; 
y2=616; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk9c'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module33 
x1=854; 
x2=973; 
y1=966; 
y2=1050; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[.6 .6 .6],'EdgeColor','b','LineWidth',1);
str = 'bk9d'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module7 
x1=1056; 
x2=1196; 
y1=0; 
y2=497; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.800000 0.300000 0.100000],'EdgeColor','b','LineWidth',1);
str = 'bk13'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%module4 
x1=1414; 
x2=1533; 
y1=616; 
y2=665; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[0.800000 0.300000 0.100000],'EdgeColor','b','LineWidth',1);
str = 'bk10c'; 
 nstr = strrep(str,'_',' '); 
 text(x1+5,y2-((y2-y1)/4),nstr);%region 
x1=874; 
x2=2074; 
y1=41; 
y2=1241; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor','none','EdgeColor',[0.800000 0.300000 0.100000],'LineWidth',1,'LineStyle','--');
%region 
x1=0; 
x2=1797; 
y1=497; 
y2=498; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','r','LineWidth',1);
%region 
x1=560; 
x2=561; 
y1=0; 
y2=1797; 
rectangle('Position',[x1,y1,x2-x1,y2-y1],'FaceColor',[1 1 1],'EdgeColor','r','LineWidth',1);
