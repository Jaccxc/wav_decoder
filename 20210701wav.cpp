#include <iostream>
#include <fstream>
#include <math.h>

#define WAV_CHANNEL 0x16
#define WAV_FREQ 0x18
#define WAV_BPS 0x1c
#define WAV_SAMPLE_DEPTH 0x22
#define WAV_DATA_SIZE 0x28
#define WAV_DATA 0x2c

using namespace std;

template <typename Word> //用template方便各類使用
  std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
  {
    for (; size; --size, value >>= 8) //Bitwise操作 以此達成單個Byte輸出
      outs.put( static_cast <char> (value & 0xFF) ); //用And 1111 1111, 取得單個Byte後 用char輸出
    return outs;
  }

struct wav_struct
{
    unsigned long file_size; //檔案大小
    unsigned long subblock1 = 16; //區塊常數
    unsigned short PCM = 1; //壓縮常數
    unsigned short channel = 1; //頻道數
    unsigned long frequency = 44100; //採樣率
    unsigned long Bps = 176400; //每秒Byte率 由計算採樣率*通到數*樣本深度得到
    unsigned short align = 4; //對齊常數
    unsigned short sample_num_bit = 16; //樣本深度
    unsigned long data_size; //音頻資料大小 計算檔案資料大小-44得到
    unsigned char *data; //音頻資料

};


wav_struct read_wav(string filename){

    fstream fs;
    wav_struct WAV;
    fs.open(filename.c_str() , ios::binary|ios::in);

    fs.seekg(0,ios::end);
    WAV.file_size=fs.tellg();

    fs.seekg(WAV_CHANNEL); //資料指針移動到指定偏移量處
    fs.read((char*)&WAV.channel,sizeof(WAV.channel)); //讀取資料

    fs.seekg(WAV_FREQ); //資料指針移動到指定偏移量處
    fs.read((char*)&WAV.frequency,sizeof(WAV.frequency)); //讀取資料
    //下略...
    fs.seekg(WAV_BPS);
    fs.read((char*)&WAV.Bps,sizeof(WAV.Bps));

    fs.seekg(WAV_SAMPLE_DEPTH);
    fs.read((char*)&WAV.sample_num_bit,sizeof(WAV.sample_num_bit));

    WAV.Bps = WAV.frequency*WAV.channel*WAV.sample_num_bit/8; //計算採樣率*通到數*樣本深度 得出每秒Byte率

    WAV.data_size = WAV.file_size-44; //計算檔案大小減去header(44 Byte) 得到音頻資料大小

    WAV.data=new unsigned char[WAV.data_size]; //動態分配記憶體給data

    fs.seekg(WAV_DATA);
    fs.read((char *)WAV.data,sizeof(char)*WAV.data_size); //讀取資料

    fs.close();

    return WAV;

}

void write_wav(string filename, wav_struct WAV){

    ofstream f( "Output.wav", ios::binary );

    f << "RIFF----WAVEfmt "; //輸出常數標幟 中間檔案大小留到最後計算補上
    //根據wav結構 順序輸出各參數
    write_word( f, WAV.subblock1, 4 );
    write_word( f, WAV.PCM, 2);
    write_word( f, WAV.channel, 2 );
    write_word( f, WAV.frequency, 4 );
    write_word( f, WAV.Bps, 4 );
    write_word( f, WAV.align, 2 );
    write_word( f, WAV.sample_num_bit, 2 );

    //紀錄音頻資料起點位置
    size_t pos_data = f.tellp();

    f << "data----"; //輸出常數標幟 中間音頻資料大小留到最後計算補上

    //輸出已存在結構中的音頻資料
    for(int i=0; i<WAV.data_size; i++){
        write_word( f, WAV.data[i], 1);
    }


    size_t file_length = f.tellp(); //記錄檔案結尾位置
    f.seekp( pos_data + 4 ); //回到音頻資料起點
    write_word( f, file_length - pos_data + 8 ); //補上音頻資料大小

    f.seekp( 0 + 4 ); //回到檔案起點
    write_word( f, file_length - 8, 4 ); //計算檔案結尾-8 補上檔案大小


}

int main(int argc,char **argv)
{
    string filename;
    cout << "Enter the filename: (including .wav): ";
    cin >> filename;

    wav_struct WAV = read_wav(filename); //從檔案建立WAV結構檔案
    int mode;
    double factor;

    cout<<"File_size = "<<WAV.file_size<<endl;
    cout<<"Channel_number = "<<WAV.channel<<endl;
    cout<<"Sampling_frequency = "<<WAV.frequency<<endl;
    cout<<"Bitrate = "<<WAV.Bps<<endl;
    cout<<"Sampling_depth = "<<WAV.sample_num_bit<<endl;
    cout<<"Data_size = "<<WAV.data_size<<endl<<endl;

    cout<< filename << " loaded." << endl << endl << "Select the function: " << endl << endl;
    cout<< "1.Speed up " << endl;
    cout<< "2.Slow down  " << endl;
    cin >> mode;

    switch (mode) {
        case 1:
            cout<< "Enter the factor (1 to 2, ex: 1.6)" << endl;
            cin >> factor;
            WAV.frequency = (double)WAV.frequency*factor; //更新採樣率 乘上因子
            WAV.Bps = WAV.frequency*WAV.channel*WAV.sample_num_bit/8; //更新Byte率 計算公式在struct內
            break;
        case 2:
            cout<< "Enter the factor (0 to 1, ex: 0.5)" << endl;
            cin >> factor;
            cout << "Before" << WAV.frequency <<  " " << WAV.Bps << endl;
            WAV.frequency = (double)WAV.frequency*factor;
            WAV.Bps = WAV.frequency*WAV.channel*WAV.sample_num_bit/8;
            cout << "After" << WAV.frequency <<  " " << WAV.Bps << endl;
            break;

    }

    cout<< "Done!, file name == \"Output.wav\"";
    write_wav(filename, WAV);

    delete[] WAV.data;
    system("pause");

}
