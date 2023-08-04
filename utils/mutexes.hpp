#pragma once
struct Mutexes
{
pthread_mutex_t mtx_connection_established;
pthread_mutex_t mtx_start_communicating;
pthread_cond_t cond_successful_connection;
pthread_cond_t cond_start_signal;
int num_successful_connections = 0;


//int sending_rounds = 0;
int receiving_rounds = 0;
pthread_mutex_t mtx_receive_next;
pthread_cond_t cond_receive_next;

std::vector<int> sockets_received;
pthread_mutex_t mtx_data_received;
pthread_cond_t cond_data_received;

    
int sending_rounds = 0;
pthread_mutex_t mtx_send_next;
pthread_cond_t cond_send_next;

int sockets_sent = 0;
pthread_mutex_t mtx_data_sent;
pthread_cond_t cond_data_sent;

Mutexes()
{
    sockets_received.push_back(0);
}

void init_muetexes()
{
pthread_mutex_init(&mtx_connection_established, NULL);
pthread_mutex_init(&mtx_start_communicating, NULL);
pthread_mutex_init(&mtx_send_next, NULL);
pthread_cond_init(&cond_successful_connection, NULL);
pthread_cond_init(&cond_start_signal, NULL);
pthread_cond_init(&cond_send_next, NULL);

}

void wait_for_connection()
{
    pthread_mutex_lock(&mtx_connection_established);
    while (num_successful_connections < 2 * (num_players -1)) {
    pthread_cond_wait(&cond_successful_connection, &mtx_connection_established);
    }
    num_successful_connections = -1; 
    pthread_cond_broadcast(&cond_start_signal); //signal all threads to start receiving
    pthread_mutex_unlock(&mtx_connection_established);
}

    // manual send
void manual_send()
{
    sb = 0;      
    pthread_mutex_lock(&mtx_send_next); 
     sending_rounds +=1;
      pthread_cond_broadcast(&cond_send_next); //signal all threads that sending buffer contains next data
      pthread_mutex_unlock(&mtx_send_next); 
}

void manual_receive()
{
    // manual receive

    rounds+=1;  
        // receive_data
      //wait until all sockets have finished received their last data
    pthread_mutex_lock(&mtx_receive_next);
      
    while(rounds > receiving_rounds) //wait until all threads received their data
          pthread_cond_wait(&cond_receive_next, &mtx_receive_next);
      
    pthread_mutex_unlock(&mtx_receive_next);

    rb = 0;
}

void reset()
{
num_successful_connections = 0;
std::fill(sockets_received.begin(), sockets_received.end(), 0);
share_buffer[0] = 0;
share_buffer[1] = 0;
send_count[0] = 0;
send_count[1] = 0;
rb = 0;
sb = 0;
rounds = 0;
sending_rounds = 0;
receiving_rounds = 0;
}


};
