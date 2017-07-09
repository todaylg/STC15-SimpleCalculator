/*
    基于IAP15F2K61S2单片机及QC12864B液晶显示屏实现简易计算器
*/
#include<reg51.h>   
#define uint unsigned int 
#define uchar unsigned char
#define ulong unsigned long

sbit rs=P1^0;//并行的指令/数据选择信号
sbit rw=P1^1;//并行的读/写选择信号
sbit e=P1^2;//控制引脚(并行的使能信号)
sbit res=P1^3;
sbit psb=P1^6;//串并选择，H=串 L=并

char i,j,temp,num,num_1;
long a,b,c;     //a,第一个数 b,第二个数 c,得数
float a_c,b_c;
uchar symbolFlag,symbolValue;
//symbolFlag=1表示有运算符键按下，symbolFlag=0表示没有运算符键按下；
//symbolValue=1为加法，symbolValue=2为减法，symbolValue=3为乘法，symbolValue=4为除法。

uchar code table[]={//运算数字输入数组
7,8,9,0,
4,5,6,0,
1,2,3,0,
0,0,0,0};

uchar code table1[]={        //经处理后进行键输入显示准备的数组 (ASCII Key Value)
7,8,9,0x2f-0x30,        //7，8，9，÷     
4,5,6,0x2a-0x30,        //4, 5, 6，×
1,2,3,0x2d-0x30,        //1, 2, 3，－
0x01-0x30,0,0x3d-0x30,0x2b-0x30//C，0，=，＋
};

long table2[19];     //存储结果的数组(20报错)
uchar code beginWord[]="This is a simple";
uchar code beginWord1[]="Calculator" ;

//延时函数
void delay(uint x){//x单位为毫秒
    uint i,j;
    for(i=x;i>0;i--)
        for(j=110;j>0;j--);
}

//写指令
void write_com(uchar com){
    rs=0;                           
    rw=0;
    e=0;
    P0=com;
    delay(5);
    e=1;
    delay(5);
    e=0;                                   
}
//写数据
void write_data(uchar data){
    rs=1;                             
    rw=0;
    e=0;
    P0=data;
    delay(5);
    e=1;
    delay(5);
    e=0;
}

//初始化
void init(){
    psb=1;           //并口传输方式
    num=-1;
    e=1;rw=0;num_1=0;i=0;j=0;a=0;b=0;c=0;symbolFlag=0;symbolValue=0;
    res=0;res=1;              
    delay(50);           //先等待50个毫秒
    write_com(0x30);       //基本操作指令(ASCII=>0)
    delay(5);
    write_com(0x0e);     //显示状态开/关,08什么也没有；0c无光标；0e有光标但不闪；0f有光标闪烁
    delay(5);
    write_com(0x01);       //清除显示
    delay(5);
    write_com(0x06);       //进入点设置
    delay(5);
}

//开机屏幕显示
void begin(){
    uchar num;
    write_com(0x90);    ////80 90 88 98
    delay(2);
    for(num=0;num<16;num++){
        write_data(beginWord[num]);
        delay(2);
    }
    write_com(0x88);
    delay(2);
    for(num=0;num<10;num++){
        write_data(beginWord1[num]);
        delay(2);
    }
    delay(2000);
    write_com(0x01);//2s后清除显示
}

//键盘扫描程序
void keyscan() { 
    //第一行
    P2=0xfe; 
    if(P2!=0xfe){
        delay(20);// 延迟20ms
        if(P2!=0xfe) {  
            temp=P2&0xf0;
            switch(temp){
                case 0xe0:num=0;   break;        //7    1110
                case 0xd0:num=1;   break;        //8    1101
                case 0xb0:num=2;   break;        //9    1011
                case 0x70:num=3;   break;        //÷    0111
            }
        }
        while(P2!=0xfe);//确保松开了后
        if(num==0||num==1||num==2){//如果按下的是'7','8'或'9
            if(j!=0){
                write_com(0x01); j=0;//j代表是否按下了=，在下一次计算之前先清屏
            }
            if(symbolFlag==0){//没有按过运算符键
                a=a*10+table[num];//按下数字存储到a
            }else{//如果按过运算符键
                b=b*10+table[num];//按下数字存储到b
            }
        }else{//如果按下的是'/'（除法）
            symbolFlag=1;         //按下了运算符
            symbolValue=4;//4表示除号已按
        }
        i=table1[num];     //数据显示做准备
        write_data(0x30+i);//显示数据或操作符号 都是以0的ASCII key：30为基准
    }
    //第二行
    P2=0xfd; 
    if(P2!=0xfd){
        delay(20);// 延迟20ms
        if(P2!=0xfd) {  
            temp=P2&0xf0;
            switch(temp){
                case 0xe0:num=4; break; //4   
                case 0xd0:num=5; break; //5
                case 0xb0:num=6; break; //6
                case 0x70:num=7; break; //×
            }
        }
        while(P2!=0xfd);//确保松开了后
        if(num==4||num==5||num==6){//如果按下的是'4','5'或'6'
            if(j!=0){
                write_com(0x01); j=0;
            }
            if(symbolFlag==0){//没有按过运算符键
                a=a*10+table[num];//按下数字存储到a
            }else{//如果按过运算符键
                b=b*10+table[num];//按下数字存储到b
            }
        }else{//如果按下的是'×'
            symbolFlag=1;         //按下了运算符
            symbolValue=3;
        }
        i=table1[num];     //数据显示做准备
        write_data(0x30+i);//显示数据或操作符号
    }
    //第三行
    P2=0xfb; 
    if(P2!=0xfb){
        delay(20);// 延迟20ms
        if(P2!=0xfb) {  
            temp=P2&0xf0;
            switch(temp){
                case 0xe0:num=8;    break;         //1
                case 0xd0:num=9;    break;         //2
                case 0xb0:num=10;   break;         //3
                case 0x70:num=11;   break;         //-
            }
        }
        while(P2!=0xfb);//确保松开了后
        if(num==8||num==9||num==10){//如果按下的是'1','2'或'3'
            if(j!=0){
                write_com(0x01); j=0;
            }
            if(symbolFlag==0){//没有按过运算符键
                a=a*10+table[num];//按下数字存储到a
            }else{//如果按过运算符键
                b=b*10+table[num];//按下数字存储到b
            }
        }else{//如果按下的是'-'
            symbolFlag=1;         //按下运算符
            symbolValue=2;
        }
        i=table1[num];     //数据显示做准备
        write_data(0x30+i);//显示数据或操作符号
    }  
    //第四行
    P2=0xf7; 
    if(P2!=0xf7){
        delay(20);// 延迟20ms
        if(P2!=0xf7) {  
            temp=P2&0xf0;
            switch(temp){
                case 0xe0:num=12; break;  //清0键    
                case 0xd0:num=13; break;  //数字0    
                case 0xb0:num=14; break;  //等于键    
                case 0x70:num=15; break;  //加
            }
        }
        while(P2!=0xf7);//确保松开了后
        switch(num){
            case 12:{//按下的是"清零"
                write_com(0x01);
                a=0;
                b=0;
                symbolFlag=0;
                symbolValue=0;
            }break;
            case 13:{//按下的是"0"
                if(symbolFlag==0){
                    a=a*10;
                    write_data(0x30);
                    P2=0;
                }else if(symbolFlag>=1){
                    b=b*10;
                    write_data(0x30);       
                }
            }break;
            case 14:{//按下等于键，根据运算符号进行不同的算术处理
                j=1;//j是按下等于后的标志位 
                if(symbolValue==1){//加法运算
                    write_com(0x98);           //结果在第四行显示
                    // write_com(0x04);     //设置从后住前写数据，每写完一个数据，光标后退一格 //游标先不管       
                    c=a+b;
                    while(c!=0){write_data(0x30+c%10);        c=c/10;                } 
                    write_data(0x3d);     //再写"="
                    a=0;b=0;symbolFlag=0;symbolValue=0;
                }
                else if(symbolValue==2){//减法运算
                    write_com(0x98);          
                    // write_com(0x04);      
                    if(a-b>0) c=a-b;
                    else c=b-a;
                    while(c!=0){
                        write_data(0x30+c%10);
                        c=c/10;         } 
                    if(a-b<0) write_data(0x2d);
                    write_data(0x3d);     //再写"="           
                    a=0;b=0;symbolFlag=0;symbolValue=0;//归位
                }
                else if(symbolValue==3){//乘法运算 
                    write_com(0x98);          
                    // write_com(0x04);      
                    c=a*b;
                    while(c!=0){
                        write_data(0x30+c%10);
                        c=c/10;
                    } 
                    write_data(0x3d);
                    a=0;b=0;symbolFlag=0;symbolValue=0;
                }
                else if(symbolValue==4){//除法运算        
                     write_com(0x98);          
                    // write_com(0x04);      
                    i=0;
                    if(b!=0){
                        c=(long)(((float)a/b)*1000);
                        while(c!=0){ 
                            write_data(0x30+c%10);
                            c=c/10;
                            i++; 
                            if(i==3) write_data(0x2e);
                        }
                        if(a/b<=0){
                            if(i<=2){                  
                                if(i==1) write_data(0x30);
                                write_data(0x2e);//  .
                                write_data(0x30);
                            } 
                                             
                            write_data(0x30);//
                        }
                        write_data(0x3d);                              
                        a=0;b=0;symbolFlag=0;symbolValue=0;
                    }else{//error
                        write_data('!');write_data('R');write_data('O');
                        write_data('R');write_data('R');write_data('E');
                    }
                }
            }break;
            case 15:{
                write_data(0x30+table1[num]);
                symbolFlag=1;
                symbolValue=1;//加键  设置加标志symbolValue=1;
            }break;        
        }
    }
}  

void main(){
    init();
    delay(5);
    begin();    //开机初始化
    delay(5);
    write_com(0x80);    //输入数从第一行开始显示
    while(1){
        keyscan();  //键盘扫描
    }
}
