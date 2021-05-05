#include "Workload.h"

Workload::Workload() {}

bool Workload::runTime(chrono::steady_clock::time_point& startTime, int duration){
    if(chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count() <= duration)
        return true;
    else
        return false;
}

void Workload::AnalyticalStream(AnalyticalClient* aClient, runType& typeOfRun, std::atomic<int>& freshness){
    int q;
    int iso = 14;
    int con =13;
    int ret = -1;
    chrono::steady_clock::time_point startTime;
    chrono::high_resolution_clock::time_point  execTimeStart;
    long  execTimeEnd;
    if(typeOfRun == warmup) {
        q = DataSrc::uniformIntDist(0, 12);
        startTime = chrono::steady_clock::now();
        while (runTime(startTime, UserInput::getWarmUpDuration())==true) {
       	    aClient->ExecuteQuery(iso);
	    aClient->ExecuteQuery(con);
            ret = aClient->ExecuteQuery(q);
            if (q == 12)
                q = 0;
            else
                q++;
        }
    }
    else if(typeOfRun == testing){
        q = DataSrc::uniformIntDist(0, 12);
        startTime = chrono::steady_clock::now();
        while (runTime(startTime, UserInput::getTestDuration())==true){
            //int f = freshness.load();
            //aClient->SetFreshness(f);
            execTimeStart = chrono::high_resolution_clock::now();      // start timer to measure the response time	    
	    aClient->ExecuteQuery(iso);
	    aClient->ExecuteQuery(con);
            ret = aClient->ExecuteQuery(q);
            execTimeEnd = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
            aClient->SetExecutionTime(execTimeEnd, q);
            if(ret == 0)
                aClient->IncrementQueriesNum();
            if(q==12)
                q=0;
            else
                q++;
        }
    }
}

void Workload::TransactionalStreamPS(TransactionalClient* tClient, Globals* g, SQLHDBC& dbc){
    int p;
    int loOrderKey;
    chrono::steady_clock::time_point startTime;
    chrono::high_resolution_clock::time_point  execTimeStart;
    cout << "You chosed PREPARED STATEMENTS" << endl;
    long  latency;
    if(UserInput::getdbChoice() == tidb){
        if(g->typeOfRun == warmup) {
            startTime = chrono::steady_clock::now();
            while (runTime(startTime, UserInput::getWarmUpDuration())==true) {
                p = DataSrc::uniformIntDist(1, 100);
                if (p <= 96) {
                    loOrderKey = g->getLoOrderKey();
                    tClient->SetLoOrderKey(loOrderKey);
                    tClient->NewOrderTransaction(dbc);
                    tClient->PaymentTransaction(dbc);
                }
                else if (p > 96)
                    tClient->CountOrdersTransaction(dbc);
            }
        }
        else if(g->typeOfRun == testing){
            startTime = chrono::steady_clock::now();
            while (runTime(startTime, UserInput::getTestDuration())==true){
                p = DataSrc::uniformIntDist(1, 100);
                if (p <= 96) {
                    loOrderKey = g->getLoOrderKey();
                    tClient->SetLoOrderKey(loOrderKey);
                    execTimeStart = chrono::high_resolution_clock::now();       // measure response time of the tran_type 1
                    tClient->NewOrderTransaction(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    tClient->SetLatency(latency, 1);
                    tClient->IncrementTransactionNum();
                    execTimeStart = chrono::high_resolution_clock::now();;      // measure response time of the tran_type 2
                    tClient->PaymentTransaction(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    tClient->SetLatency(latency, 2);
                    tClient->IncrementTransactionNum();
                } else if (p > 96) {
                    execTimeStart = chrono::high_resolution_clock::now();      // measure response time of the tran_type 3
                    tClient->CountOrdersTransaction(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    tClient->SetLatency(latency , 3);
                    tClient->IncrementTransactionNum();
                } 
                /* if(g->getBatch() == UserInput::getBatchSize()){
                    g->batch = 0;
                    tClient->FreshnessTransaction(dbc);
                    g->freshness++;
                }*/
            }
        }

    }

}

void Workload::TransactionalStreamSP(TransactionalClient* tClient, Globals* g, SQLHDBC& dbc){
    int p;
    int loOrderKey;
    int ret_no, ret_p, ret_co;
    chrono::steady_clock::time_point startTime;
    chrono::high_resolution_clock::time_point  execTimeStart;
    cout << "You chosed STORED PROCEDURES" << endl;
    long  latency;
    if(UserInput::getdbChoice() == postgres){
        if(g->typeOfRun == warmup) {
            startTime = chrono::steady_clock::now();
            while (runTime(startTime, UserInput::getWarmUpDuration())==true) {
                p = DataSrc::uniformIntDist(1, 100);
                if (p <= 96) {
                    loOrderKey = g->getLoOrderKey();
                    tClient->SetLoOrderKey(loOrderKey);
                    tClient->NewOrderTransactionPS(dbc);
                    tClient->PaymentTransactionSP(dbc);
                }
                else if (p > 96)
                    tClient->CountOrdersTransactionSP(dbc);
            }
        }
        else if(g->typeOfRun == testing){
            startTime = chrono::steady_clock::now();
            while (runTime(startTime, UserInput::getTestDuration())==true){
                p = DataSrc::uniformIntDist(1, 100);
                if (p <= 96) {
                    loOrderKey = g->getLoOrderKey();
                    tClient->SetLoOrderKey(loOrderKey);
                    execTimeStart = chrono::high_resolution_clock::now();       // measure response time of the tran_type 1
                    ret_no = tClient->NewOrderTransactionPS(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    if(ret_no == 1){
                        tClient->SetLatency(latency, 1);
                        tClient->IncrementTransactionNum();
                    }
                    execTimeStart = chrono::high_resolution_clock::now();;      // measure response time of the tran_type 2
                    ret_p = tClient->PaymentTransactionSP(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    if(ret_p == 1){
                        tClient->SetLatency(latency, 2);
                        tClient->IncrementTransactionNum();
                    }
                    
                } else if (p > 96) {
                    execTimeStart = chrono::high_resolution_clock::now();      // measure response time of the tran_type 3
                    ret_co = tClient->CountOrdersTransactionSP(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    if(ret_co == 1){
                        tClient->SetLatency(latency , 3);
                        tClient->IncrementTransactionNum();
                    }
                } 
                /*if(g->getBatch() == UserInput::getBatchSize()){
                    g->batch = 0;
                    tClient->FreshnessTransactionSP(dbc);
                    g->freshness++;
                }*/
            }
        }

    }
    else if(UserInput::getdbChoice() == systemx){
        if(g->typeOfRun == warmup) {
            startTime = chrono::steady_clock::now();
            while (runTime(startTime, UserInput::getWarmUpDuration())==true) {
                p = DataSrc::uniformIntDist(1, 100);
                if (p <= 96) {
                    loOrderKey = g->getLoOrderKey();
                    tClient->SetLoOrderKey(loOrderKey);
                    tClient->NewOrderTransactionSS(dbc);
                    tClient->PaymentTransactionSP(dbc);
                }
                else if (p > 96)
                    tClient->CountOrdersTransactionSP(dbc);
            }
        }
        else if(g->typeOfRun == testing){
            startTime = chrono::steady_clock::now();
            while (runTime(startTime, UserInput::getTestDuration())==true){
                p = DataSrc::uniformIntDist(1, 100);
                if (p <= 96) {
                    loOrderKey = g->getLoOrderKey();
                    tClient->SetLoOrderKey(loOrderKey);
                    execTimeStart = chrono::high_resolution_clock::now();       // measure response time of the tran_type 1
                    ret_no = tClient->NewOrderTransactionSS(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    if(ret_no == 1){
                        tClient->SetLatency(latency, 1);
                        tClient->IncrementTransactionNum();
                    }
                    execTimeStart = chrono::high_resolution_clock::now();;      // measure response time of the tran_type 2
                    ret_p = tClient->PaymentTransactionSP(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    if(ret_p == 1){
                        tClient->SetLatency(latency, 2);
                        tClient->IncrementTransactionNum();
                    }
                } else if (p > 96) {
                    execTimeStart = chrono::high_resolution_clock::now();      // measure response time of the tran_type 3
                    ret_co = tClient->CountOrdersTransactionSP(dbc);
                    latency = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - execTimeStart).count();
                    if(ret_co == 1){
                        tClient->SetLatency(latency , 3);
                        tClient->IncrementTransactionNum();
                    }
                }
                /*if(g->getBatch() == UserInput::getBatchSize()){
                    g->batch = 0;
                    tClient->FreshnessTransactionSP(dbc);
                    g->freshness++;
                }*/

            }
        }

    }
}

void Workload::AnalyticalWorkload(AnalyticalClient* aClient, SQLHENV& env, Globals* g){
    aClient->SetThreadNum(this_thread::get_id());
    chrono::steady_clock::time_point startTest;
    long endTest;
    SQLHDBC dbc = 0;
    Driver::connectDB2(env, dbc);
    aClient->PrepareAnalyticalStmt(dbc);     // prepare the stmt for all the 13 queries once in the beginning
    g->barrierW->wait();
    if(g->typeOfRun == warmup)
        AnalyticalStream(aClient, g->typeOfRun, g->freshness);
    cout << "[Analytical] Warm-up is done for thread: " << aClient->GetThreadNum() <<  endl;
    g->barrierT->wait();
    if(g->typeOfRun == testing){
        startTest = chrono::steady_clock::now();
        AnalyticalStream(aClient, g->typeOfRun, g->freshness);
    }
    endTest = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTest).count();
    aClient->SetTestDuration(endTest);
    cout << "[Analytical] Testing is done for thread: " << aClient->GetThreadNum() << endl;
    aClient->FreeQueryStmt();
    Driver::disconnectDB(dbc);
    cout << "Total number of queries: " << aClient->GetQueriesNum() << endl;
    cout << "Duration of analytical testing: " << endTest << endl;

}

void Workload::TransactionalWorkload(TransactionalClient* tClient, SQLHENV& env, Globals* g, int t) {
    tClient->SetThreadNum(this_thread::get_id());
    SQLHDBC dbc = 0;
    chrono::steady_clock::time_point startTest;
    long endTest;
    Driver::connectDB2(env, dbc);
    tClient->SetClientNum(t);

    if(UserInput::getExecType() == ps) {
        tClient->PrepareTransactionStmt(dbc);
        g->barrierW->wait();
        if(g->typeOfRun == warmup)
            TransactionalStreamPS(tClient, g, dbc);
        cout << "[Tran] Warm-up is done for thread: " << tClient->GetThreadNum() <<  endl;
        g->barrierT->wait();
        if(g->typeOfRun == testing) {
            startTest = chrono::steady_clock::now();    // start timer
            TransactionalStreamPS(tClient, g, dbc);
        }
        tClient->FreeTransactionStmt();
    }

    else if(UserInput::getExecType() == sp){        
        g->barrierW->wait();
        if(g->typeOfRun == warmup)
            TransactionalStreamSP(tClient, g, dbc);
        cout << "[Tran] Warm-up is done for thread: " << tClient->GetThreadNum() <<  endl;
        g->barrierT->wait();
        if(g->typeOfRun == testing) {
            startTest = chrono::steady_clock::now();    // start timer
            TransactionalStreamSP(tClient, g, dbc);
        }
    }

    endTest = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTest).count();
    cout << "[Transactions] Testing is done for thread: " << tClient->GetThreadNum() << endl;
    Driver::disconnectDB(dbc);
    cout << "Total number of transactions: " << tClient->GetTransactionNum() << endl;
    cout << "Duration of transactional testing: " << endTest << endl;
}

void Workload::ExecuteWorkloads(SQLHENV& env, Globals* g) {
    for (int i = 0; i < UserInput::getAnalClients(); i++) {
        aClients.push_back(new AnalyticalClient());
        aThreads.push_back(thread(AnalyticalWorkload, aClients[i], ref(env), g));
    }

    for(int i=0; i<UserInput::getTranClients(); i++){
        tClients.push_back(new TransactionalClient());
        tThreads.push_back(thread(TransactionalWorkload, tClients[i], ref(env), g, i+1));
    }

    g->typeOfRun = warmup;
    sleep(UserInput::getWarmUpDuration());
    g->typeOfRun = testing;
    sleep(UserInput::getTestDuration());
    g->typeOfRun = none;

    for(int i=0; i<UserInput::getAnalClients(); i++)
        aThreads[i].join();

    for(int i=0; i<UserInput::getTranClients(); i++)
        tThreads[i].join();
}

void Workload::ReturnResults(Results *r) {
    r->computeResults(tClients, aClients);
}