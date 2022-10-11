helpers.h:
    - fisierul header unde am declarat structurile folosite;
    - contine si defineurile utilizate.

server.cpp:
    - reprezinta implementarea serverului;
    - creeam scoketii udp si tcp;
    - asociam adresele cu socketii;
    - ascultam pentru socketul TCP;
    - dezactivam algoritmul lui nagle;
    - adaugam scoketii udp, tcp si cel de stdin in multimea de citire;
    - asteptam conexiunea unui client;
    - parcurgem clientii si verificam care sunt in lista de desriptori;
    - verificam daca primim comanda de exit si inchidem severul;
    - verificam daca socketul este cel de udp;
    - primim mesajul de la clientul udp, verificam conditiile si formam mesajul tcp;
    - dupa formarea mesajelor cautam topicul dorit, clientii activi si le trimitem mesajele;
    - daca acestia sunt inacitvi verificam daca vor sa primeasca mesajele la repornire si adaugam mesajele in coada;
    - verificam daca socketul este cel de tcp;
    - citim de pe socket;
    - dezactivam algoritmul lui nagle;
    - adaugam soscketul in lista de descriptori;
    - primim idul de la clientul tcp;
    - parcurgem clientii verificam daca este idul dorit;
    - daca clientul este deja activat afisam mesajul dorit si inchidem noul socket;
    - altfel reconectam clientulsi afisam mesajele in asteptare;
    - daca clientul nu exista il creeam, il adaugam la topicul dorit;
    - daca este alt socket primim mesajul de la clientul tpc;
    - daca nu s-a primit nimic, dezactivam socketul si-l scoatem din lista de descriptori;
    - inchidem clientul;
    - pentru comanda de subscribe cautam topicul dorit;
    - verificam dac acesta exista, daca nu il creeam;
    - altfel adaugam in cel dorit si setam storeandforward;
    - pentru comanda de unsubscribe scoatem clientul din topic.

subscribe.cpp:
    - creeam socketul;
    - completam adresele;
    - clientul se connecteaza la server;
    - trimitem idul serverului;
    - dezactivam algortimul lui neagle;
    - se adauga descriptorii la lista;
    - modificam lista de descriptori cu select si asteapta un socket pentru interactiune;
    - verificam daca socketul de citire apartine listei de descriptori;
    - citim comenzile de la tastatura;
    - daca aceasta este exit inchidem clientul;
    - altfel trimitem comenzile catre server pentru a se efectua modificarile dorite;
    - afisam mesajele dorite;
    - verificam daca socketul care comunica cu serverul se afla in lista;
    - primim mesajul de la server si verificam marimea sa;
    - afisam mesajul trimis de la clientul UDP.

Mentiuni:
    - am utilizat laboratoarele 6, 7 si 8 in rezolvarea temei ca suport.
    