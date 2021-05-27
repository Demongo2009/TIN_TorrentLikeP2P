//
// Created by konrad on 5/27/21.
//

#include <thread>

#include "../include/TorrentClient.h"

void TorrentClient::run() {
    /**
     * 1.init struktur
     * 2.pthread_create(runServerThread)
     * 3.sleep(1sec)? - zeby poczekac chwilkę żebyśmy zdazyli sfetchowac stan sieci, mozemy to tez w wątku klienta przy czym wysietlimy jakas informacje typu "sekunda... inicjlaizacja węzła"
     * 4.phread_creat(runCliThread)
     */
}

void TorrentClient::init() {
 /**
  * inicjalizacja struktur
  * */
}

void *TorrentClient::runServerThread() {
    /**
     * na początku: rozglos ze jestes nowy
     *
     * while(true){
     *  tutaj typowy schemat pracy serwera(z uxpow mozna przekopiowac), czyli:
     *      1. nasłuchuj na połączenie/jakiś komunikat
     *      2. zobacz jaki to komunikat
     *      3. wywolaj odpowiednią funkcję w zależności od rodzaju komunikatu
     *      4. wróc do nasłuchiwania
     * }
     * */

    return nullptr;
}

void *TorrentClient::runCliThread() {
    /**
     * 1. wypisz ze "trwa inicjalizacja" -> sleep(2s)
     * while(true){
     *  tutaj tez typowa pętla interfejsu, czyli:
     *      1. pobierz polecenie od uzytkownika
     *      2. oddeleguj odpowiedni wątek
     *      3. wróc do pobierania poleceń
     * }
     * */
    return nullptr;
}


