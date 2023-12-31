#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<cstring>
#include<time.h>
#include"Neural.h"

using namespace std;

const int input_neurons=2;
int number_of_hidden_layers=5;
const int hidden_layers_neurons=8;
int output_neurons = 1;
float input_data[input_neurons]={0.1,0.2};
float INPUT[hidden_layers_neurons]={0.1,-0.2,0.3,0.1,-0.2,0.3,0.1,-0.2};

float middle_input[hidden_layers_neurons][hidden_layers_neurons]={
    {-0.2,0.3,-0.4,0.5,0.6,-0.7,0.8,-0.9},
    {0.2,-0.3,0.4,-0.5,-0.6,0.7,-0.8,0.9},
    {0.3,-0.4,0.5,-0.6,-0.7,0.8,-0.9,0.1},
    {0.4,-0.5,0.6,-0.7,-0.8,0.9,-0.1,0.2},
    {0.5,-0.6,0.7,-0.8,-0.9,0.1,-0.2,0.3}};
int fd[2];
int fd2[2];
int fd3[2];
int outputpipe[2];
char buffer[1024];
pthread_mutex_t m1;
pthread_mutex_t m2,m3;
float val;
float m_val;
float sec=0;
float OUTPUT=0;
float requiredoutput = 0.1;

float x1 = 0, x2 = 0;
int count = 0;

bool hidden_layers();
bool output_layers();


void*fun(void*args)
{
    float input=0;
    input_attr* i1 = (input_attr*)args;
    int j = i1->ith_value;
    
    input = i1->input_weights[j];
    float input_layer_product = 0;
    pthread_mutex_init (&m1 ,NULL);
    pthread_mutex_lock (&m1);
    for(int i=0;i<input_neurons;i++)
    {     
        input_layer_product += (input * i1->weights[i]);
    }
    
    cout<< pthread_self() << " INPUT " << input_layer_product << endl;    
    write(fd[1],&input_layer_product,sizeof(input_layer_product));

    pthread_mutex_unlock (&m1) ;

    pthread_exit(NULL);
}


void input_layer()
{

	pthread_attr_t attr;
    pthread_attr_init(&attr);
    //Set the scheduling policy to Round-Robin
    pthread_attr_setschedpolicy(&attr,SCHED_RR);
    //pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    struct sched_param ts ;
    ts.sched_priority = 0;
    pthread_attr_setschedparam (&attr,&ts);
    do
    {
    pid_t pid =fork();   

    if(pid>=0)
    {
        if(pid==0)
        {
            if(!(x1 == 0 && x2 == 0))
            {
                input_data[0] = x1;
                input_data[1] = x2;
                cout << "At input layer" << endl;
                cout << "Error is : \n f(x1) = " << x1 << endl;
                cout << "f(x2) = " << x2 << endl;
            }
           
            pthread_t p1[hidden_layers_neurons];
            //pthread_t p1[input_neurons]
            input_attr *in =new input_attr;
            in->weights[0]=input_data[0];
            in->weights[1]=input_data[1];
            for(int i=0;i<hidden_layers_neurons;i++)
            {
                in->input_weights[i]=INPUT[i];
            }
            for(int i=0;i<hidden_layers_neurons;i++)
            {
                in->ith_value=i;
                pthread_create(&p1[i],&attr,fun,(void*)in);
                pthread_join(p1[i], NULL);
            }
            
            
        }

    }
    }while(!hidden_layers());
    pthread_attr_destroy(&attr);
    return;
}

void*fun1(void*args)
{
    float input = 0;
    middle_attr* m1 = (middle_attr*)args;
    int j = m1->ith_value;
    middle_attr* mn = (middle_attr*)args;

    input = mn->weights[j];

    float hidden_layer_product = 0;
    for(int i=0;i<hidden_layers_neurons;i++)
    {     
        hidden_layer_product += (input * m1->input[i]);
    }

    cout << "hidden layer" << m1->p_value << " " << m1->ith_value << " " << hidden_layer_product << endl;
   // cout<<m1->ith_value<<" : "<<(number_of_hidden_layers + 1)<<endl;
   /* if(m1-> ith_value == (number_of_hidden_layers + 1))
    {
    	
        cout << "last hidden layer: " << m1->p_value << endl;
        write(fd3[1],&hidden_layer_product,sizeof(hidden_layer_product));      // 2nd pipe
       // cout<<"writing lasrttt"<<endl;

    }
    else{*/
    	pthread_mutex_init (&m3 ,NULL);
    	pthread_mutex_lock(&m3);
        write(fd2[1],&hidden_layer_product,sizeof(hidden_layer_product));      // 2nd pipe
        pthread_mutex_unlock(&m3);
        cout<<"writing"<<endl;
        
  //  }
    
    return NULL;
}

bool hidden_layers()
{
	pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr,SCHED_RR);
    struct sched_param ts ;
    ts.sched_priority = 0;
    pthread_attr_setschedparam (&attr,&ts);
    pid_t ids[number_of_hidden_layers];
    pthread_mutex_init (&m2 ,NULL);
	
    for(int i=0;i<number_of_hidden_layers;i++)
    { 
        ids[i]=fork();
         cout<<i<<endl;
        if(ids[i]>=0)
        {
            if(ids[i]==0)
            {
                pthread_t pids[hidden_layers_neurons];
                middle_attr* ml= new middle_attr;

                for(int j=0;j<hidden_layers_neurons;j++)
                {
                    pthread_mutex_lock(&m2);
                    if(i == 0)
                    {
                        read(fd[0], &m_val, sizeof(m_val));
                    }
                    else 
                    {
                        read(fd2[0], &m_val, sizeof(m_val));        
                    }
                    pthread_mutex_unlock(&m2);
                    cout << "input to hidden layer" << m_val << endl;
                    ml->input[j] = m_val;
                    ml->weights[j]= middle_input[i][j];
                }
				
                for(int j=0;j<hidden_layers_neurons;j++)
                {
                  //  cout<<"input here"<<i<<" : "<<j<<endl;
                    ml->ith_value=j;  
                    ml->p_value = i;            
                    pthread_create(&pids[j],&attr,fun1,(void*)ml);
                    pthread_join(pids[j], NULL);
                }
                exit(0);
            }
            wait(NULL);
        }
    }
  //  cout << "END yayyyy" << endl;
    if(!output_layers())
    {
        for(int i = number_of_hidden_layers -1; i >= 0; i-- )
        {
            cout << "Going back to layer " << i+1 << endl;
            cout << "Error is:\nf(x1) = " << x1 << endl;
            cout << "f(x2) = " << x2 << endl;
        }

        return false;
    }
    
    pthread_attr_destroy(&attr);
    return true;
}

void*fun3(void*args)
{
	pthread_mutex_t outputMu;
	pthread_mutex_init (&outputMu ,NULL);
	
    float output = 0;
    float sum = 0;
    //OUTPUT = 0;
    pthread_mutex_lock (&outputMu);
    for (int i = 0; i < hidden_layers_neurons; i++)
    {	
        read(fd2[0], &output, sizeof(output));
        sum += (output * -0.1);
        cout<<"sum:"<<output<<" "<<sum<<endl;
    }
	pthread_mutex_unlock (&outputMu);	
	cout<<"output Pipe"<<endl;
    write (outputpipe[1], &sum, sizeof(sum));

    cout<< sum <<"++"<<endl;
    ///////////condition///////////////////////////////


    pthread_exit(NULL);
}


bool output_layers()
{
	pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr,SCHED_RR);
    struct sched_param ts ;
    ts.sched_priority = 0;
    pthread_attr_setschedparam (&attr,&ts);
    pid_t foutput = fork();
    pthread_t opid[output_neurons];
    
    if(foutput>=0)
    {
        if(foutput==0)
        {
            for(int i=0;i< output_neurons;i++)
            {
                pthread_create(&opid[i],&attr,fun3,NULL);
                cout<<"creating threads for last"<<endl;
                pthread_join(opid[i], NULL);
            }

		cout<<"aa ii ai ia i  wanna give u one last optionn"<<endl;
        }
		
        read (outputpipe[0], &OUTPUT, sizeof(OUTPUT));
        cout<<"output"<<OUTPUT<<endl;
        if(count < 1 && OUTPUT != requiredoutput)
        {
            cout << "Back propogating" << endl;
            x1 = (OUTPUT + (OUTPUT * OUTPUT) + 1)/2;
            x2 = ((OUTPUT * OUTPUT) - OUTPUT)/2;
            count++;
            return false;
        }
        else{
            cout << "Correct Output" << endl;
            return true;
        } 
    }

    return false;
	pthread_attr_destroy(&attr);
}

int main()
{

    pipe(fd);
    pipe(fd2);
    pipe(fd3);
    pipe(outputpipe);
    input_layer();
    cout << "weee = " << OUTPUT << endl;
    return 0;
}
