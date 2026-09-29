#define N 100
int count = 0;

void producer(void) {
    int item;
    while (TRUE) {
        item = produce();
        if (count == N) sleep();
        insert(item);
        count = count + 1;
        if (count == 1) wakeup(consumer);
    }
}

void consumer(void) {
    int item;
    while (TRUE) {
        if (count == 0) sleep();
        item = remove_item();
        count = count - 1;
        if (count == N - 1) wakeup(producer);
        consume(item);
    }
}

/*
Step-by-Step Interleaving Scenario:Initial
 State: The buffer is empty (count == 0).
 Consumer Starts: The Consumer executes its 
 loop and checks if (count == 0). The condition
  evaluates to true.Interrupt Occurs: Right 
  after evaluating count == 0, but BEFORE 
  the Consumer actually executes sleep(),
   the CPU interrupts the Consumer and context
   -switches to the Producer.Producer Runs:The 
   Producer produces an item, inserts it into th
   e buffer, and increments count = 1.The Produc
   er checks if (count == 1) (which is true) and
    calls wakeup(consumer).Lost Signal: Because 
    the Consumer was not actually asleep yet (it
     was interrupted just before sleeping), the 
     
     
     wakeup signal is sent to an active process 
     
     is lost forever.Consumer Resumes: The Consum
     er resumes execution right where it was stop
     ped and executes sleep().Deadlock: The Produ
     cer will eventually fill all $N$ slots of th
     
     e buffer (count == N) and call sleep(). Now
      both processes are asleep forever, locked 
      in a deadlock.

*/