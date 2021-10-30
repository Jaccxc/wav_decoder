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

template <typename Word> //��template��K�U���ϥ�
  std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
  {
    for (; size; --size, value >>= 8) //Bitwise�ާ@ �H���F�����Byte��X
      outs.put( static_cast <char> (value & 0xFF) ); //��And 1111 1111, ���o���Byte�� ��char��X
    return outs;
  }

struct wav_struct
{
    unsigned long file_size; //�ɮפj�p
    unsigned long subblock1 = 16; //�϶��`��
    unsigned short PCM = 1; //���Y�`��
    unsigned short channel = 1; //�W�D��
    unsigned long frequency = 44100; //�ļ˲v
    unsigned long Bps = 176400; //�C��Byte�v �ѭp��ļ˲v*�q���*�˥��`�ױo��
    unsigned short align = 4; //����`��
    unsigned short sample_num_bit = 16; //�˥��`��
    unsigned long data_size; //���W��Ƥj�p �p���ɮ׸�Ƥj�p-44�o��
    unsigned char *data; //���W���

};


wav_struct read_wav(string filename){

    fstream fs;
    wav_struct WAV;
    fs.open(filename.c_str() , ios::binary|ios::in);

    fs.seekg(0,ios::end);
    WAV.file_size=fs.tellg();

    fs.seekg(WAV_CHANNEL); //��ƫ��w���ʨ���w�����q�B
    fs.read((char*)&WAV.channel,sizeof(WAV.channel)); //Ū�����

    fs.seekg(WAV_FREQ); //��ƫ��w���ʨ���w�����q�B
    fs.read((char*)&WAV.frequency,sizeof(WAV.frequency)); //Ū�����
    //�U��...
    fs.seekg(WAV_BPS);
    fs.read((char*)&WAV.Bps,sizeof(WAV.Bps));

    fs.seekg(WAV_SAMPLE_DEPTH);
    fs.read((char*)&WAV.sample_num_bit,sizeof(WAV.sample_num_bit));

    WAV.Bps = WAV.frequency*WAV.channel*WAV.sample_num_bit/8; //�p��ļ˲v*�q���*�˥��`�� �o�X�C��Byte�v

    WAV.data_size = WAV.file_size-44; //�p���ɮפj�p��hheader(44 Byte) �o�쭵�W��Ƥj�p

    WAV.data=new unsigned char[WAV.data_size]; //�ʺA���t�O���鵹data

    fs.seekg(WAV_DATA);
    fs.read((char *)WAV.data,sizeof(char)*WAV.data_size); //Ū�����

    fs.close();

    return WAV;

}

void write_wav(string filename, wav_struct WAV){

    ofstream f( "Output.wav", ios::binary );

    f << "RIFF----WAVEfmt "; //��X�`�Ƽмm �����ɮפj�p�d��̫�p��ɤW
    //�ھ�wav���c ���ǿ�X�U�Ѽ�
    write_word( f, WAV.subblock1, 4 );
    write_word( f, WAV.PCM, 2);
    write_word( f, WAV.channel, 2 );
    write_word( f, WAV.frequency, 4 );
    write_word( f, WAV.Bps, 4 );
    write_word( f, WAV.align, 2 );
    write_word( f, WAV.sample_num_bit, 2 );

    //�������W��ư_�I��m
    size_t pos_data = f.tellp();

    f << "data----"; //��X�`�Ƽмm �������W��Ƥj�p�d��̫�p��ɤW

    //��X�w�s�b���c�������W���
    for(int i=0; i<WAV.data_size; i++){
        write_word( f, WAV.data[i], 1);
    }


    size_t file_length = f.tellp(); //�O���ɮ׵�����m
    f.seekp( pos_data + 4 ); //�^�쭵�W��ư_�I
    write_word( f, file_length - pos_data + 8 ); //�ɤW���W��Ƥj�p

    f.seekp( 0 + 4 ); //�^���ɮװ_�I
    write_word( f, file_length - 8, 4 ); //�p���ɮ׵���-8 �ɤW�ɮפj�p


}

int main(int argc,char **argv)
{
    string filename;
    cout << "Enter the filename: (including .wav): ";
    cin >> filename;

    wav_struct WAV = read_wav(filename); //�q�ɮ׫إ�WAV���c�ɮ�
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
            WAV.frequency = (double)WAV.frequency*factor; //��s�ļ˲v ���W�]�l
            WAV.Bps = WAV.frequency*WAV.channel*WAV.sample_num_bit/8; //��sByte�v �p�⤽���bstruct��
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
