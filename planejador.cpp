#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "planejador.h"

using namespace std;

/* *************************
   * CLASSE IDPONTO        *
   ************************* */

/// Atribuicao de string
void IDPonto::set(string&& S)
{
  t=move(S);
  if (!valid()) t.clear();
}

/* *************************
   * CLASSE IDROTA         *
   ************************* */

/// Atribuicao de string
void IDRota::set(string&& S)
{
  t=move(S);
  if (!valid()) t.clear();
}

/* *************************
   * CLASSE PONTO          *
   ************************* */

/// Distancia entre 2 pontos (formula de haversine)
double haversine(const Ponto& P1, const Ponto& P2)
{
  // Tratar logo pontos identicos
  if (P1.id == P2.id) return 0.0;

  static const double MY_PI = 3.14159265358979323846;
  static const double R_EARTH = 6371.0;
  // Conversao para radianos
  double lat1 = MY_PI*P1.latitude/180.0;
  double lat2 = MY_PI*P2.latitude/180.0;
  double lon1 = MY_PI*P1.longitude/180.0;
  double lon2 = MY_PI*P2.longitude/180.0;

  double cosseno = sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1-lon2);
  // Para evitar eventuais erros na funcao acos por imprecisao numerica
  // nas operacoes com double: acos(1.0000001) eh NAN
  if ( cosseno > 1.0 ) cosseno = 1.0;
  if ( cosseno < -1.0 ) cosseno = -1.0;
  // Distancia entre os pontos
  return R_EARTH*acos(cosseno);
}

/* *************************
   * CLASSE PLANEJADOR     *
   ************************* */

/// Torna o mapa vazio
void Planejador::clear()
{
  pontos.clear();
  rotas.clear();
}

/// Retorna um Ponto do mapa, passando a id como parametro.
/// Se a id for inexistente, retorna um Ponto vazio.
Ponto Planejador::getPonto(const IDPonto& Id) const
{
  // Procura um ponto que corresponde aa Id do parametro
  /* feito */
  auto itr_pontospar = find(pontos.begin(), pontos.end(), Id);
  // Em caso de sucesso, retorna o ponto encontrado
  /* feito */
  if (itr_pontospar != pontos.end()) return *itr_pontospar;
  // Se nao encontrou, retorna um ponto vazio
  return Ponto();
}

/// Retorna um Rota do mapa, passando a id como parametro.
/// Se a id for inexistente, retorna um Rota vazio.
Rota Planejador::getRota(const IDRota& Id) const
{
  // Procura uma rota que corresponde aa Id do parametro
  /* feito */
  auto itr_rotaspar = find(rotas.begin(), rotas.end(), Id);
  // Em caso de sucesso, retorna a rota encontrada
  /* feito */
  if (itr_rotaspar != rotas.end()) return *itr_rotaspar;
  // Se nao encontrou, retorna uma rota vazia
  return Rota();
}

/// Imprime os pontos do mapa no console
void Planejador::imprimirPontos() const
{
  for (const auto& P : pontos)
  {
    cout << P.id << '\t' << P.nome
         << " (" <<P.latitude << ',' << P.longitude << ")\n";
  }
}

/// Imprime as rotas do mapa no console
void Planejador::imprimirRotas() const
{
  for (const auto& R : rotas)
  {
    cout << R.id << '\t' << R.nome << '\t' << R.comprimento << "km"
         << " [" << R.extremidade[0] << ',' << R.extremidade[1] << "]\n";
  }
}

/// Leh um mapa dos arquivos arq_pontos e arq_rotas.
/// Caso nao consiga ler dos arquivos, deixa o mapa inalterado e retorna false.
/// Retorna true em caso de leitura bem sucedida
bool Planejador::ler(const std::string& arq_pontos,
                     const std::string& arq_rotas)
{
  // Listas temporarias para armazenamento dos dados lidos
  list<Ponto> listP;
  list<Rota> listR;
  // Variaveis auxiliares para buscas nas listas
  list<Ponto>::iterator itr_ponto;
  list<Rota>::iterator itr_rota;
  // Variaveis auxiliares para leitura de dados
  Ponto P;
  Rota R;
  string prov;

  // Leh os pontos do arquivo
  try
  {
    // Abre o arquivo de pontos
    ifstream arq(arq_pontos);
    if (!arq.is_open()) throw 1;

    // Leh o cabecalho
    getline(arq,prov);
    if (arq.fail() ||
        prov != "ID;Nome;Latitude;Longitude") throw 2;

    // Leh os pontos
    do
    {
      // Leh a ID
      getline(arq,prov,';');
      if (arq.fail()) throw 3;
      P.id.set(move(prov));
      if (!P.valid()) throw 4;

      // Leh o nome
      getline(arq,prov,';');
      if (arq.fail() || prov.size()<2) throw 5;
      P.nome = move(prov);

      // Leh a latitude
      arq >> P.latitude;
      if (arq.fail()) throw 6;
      arq.ignore(1,';');

      // Leh a longitude
      arq >> P.longitude;
      if (arq.fail()) throw 7;
      arq >> ws;

      // Verifica se jah existe ponto com a mesma ID no conteiner de pontos lidos (listP)
      // Caso exista, throw 8
      /* feito */
      auto itr_idponto = find(listP.begin(), listP.end(), P.id);
      if (itr_idponto != listP.end()) throw 8;

      // Inclui o ponto na lista de pontos
      listP.push_back(move(P));
    }
    while (!arq.eof());

    // Fecha o arquivo de pontos
    arq.close();
  }
  catch (int i)
  {
    cerr << "Erro " << i << " na leitura do arquivo de pontos "
         << arq_pontos << endl;
    return false;
  }

  // Leh as rotas do arquivo
  try
  {
    // Abre o arquivo de rotas
    ifstream arq(arq_rotas);
    if (!arq.is_open()) throw 1;

    // Leh o cabecalho
    getline(arq,prov);
    if (arq.fail() ||
        prov != "ID;Nome;Extremidade 1;Extremidade 2;Comprimento") throw 2;

    // Leh as rotas
    do
    {
      // Leh a ID
      getline(arq,prov,';');
      if (arq.fail()) throw 3;
      R.id.set(move(prov));
      if (!R.valid()) throw 4;

      // Leh o nome
      getline(arq,prov,';');
      if (arq.fail() || prov.size()<2) throw 4;
      R.nome = move(prov);

      // Leh a id da extremidade[0]
      getline(arq,prov,';');
      if (arq.fail()) throw 6;
      R.extremidade[0].set(move(prov));
      if (!R.extremidade[0].valid()) throw 7;

      // Verifica se a Id corresponde a um ponto no conteiner de pontos lidos (listP)
      // Caso ponto nao exista, throw 8
      /* feito */
      auto itr_plidos = find(listP.begin(), listP.end(), R.extremidade[0]);
      if (itr_plidos == listP.end()) throw 8;

      // Leh a id da extremidade[1]
      getline(arq,prov,';');
      if (arq.fail()) throw 9;
      R.extremidade[1].set(move(prov));
      if (!R.extremidade[1].valid()) throw 10;

      // Verifica se a Id corresponde a um ponto no conteiner de pontos lidos (listP)
      // Caso ponto nao exista, throw 11
      /* feito */
      auto itr_plidos2= find(listP.begin(), listP.end(), R.extremidade[1]);
      if (itr_plidos2 == listP.end()) throw 11;

      // Leh o comprimento
      arq >> R.comprimento;
      if (arq.fail()) throw 12;
      arq >> ws;

      // Verifica se jah existe rota com a mesma ID no conteiner de rotas lidas (listR)
      // Caso exista, throw 13
      /* feito */
      auto itr_rlidas = find(listR.begin(), listR.end(), R.id);
      if (itr_rlidas != listR.end()) throw 13;

      // Inclui a rota na lista de rotas
      listR.push_back(move(R));
    }
    while (!arq.eof());

    // Fecha o arquivo de rotas
    arq.close();
  }
  catch (int i)
  {
    cerr << "Erro " << i << " na leitura do arquivo de rotas "
         << arq_rotas << endl;
    return false;
  }

  // Soh chega aqui se nao entrou no catch, jah que ele termina com return.
  // Move as listas de pontos e rotas para o planejador.
  pontos = move(listP);
  rotas = move(listR);

  return true;
}

/// *******************************************************************************
/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// *******************************************************************************

/// Noh: os elementos dos conjuntos de busca do algoritmo A*
/* feito */

class Noh {
public:
  IDPonto id_pt;  // ID do ponto
  IDRota id_rt; // ID da rota
  double g;    // Custo acumulado do ponto inicial ateh este ponto
  double h;    // Estimativa do custo do ponto ateh o ponto final

  Noh(const IDPonto& Id, const IDRota& IdR, double G, double H):
    id_pt(Id), id_rt(IdR), g(G), h(H) {}
  
  double f() const { return g + h; }

  bool operator==(const IDPonto& Id) const { return id_pt == Id; }
  bool operator==(const Noh& N) const { return id_pt == N.id_pt; }
  bool operator<(const Noh& N) const { return f() < N.f(); }
  bool operator>(const Noh& N) const { return f() > N.f(); }
};
/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// Retorna o comprimento do caminho encontrado.
/// (<0 se  parametros invalidos ou nao existe caminho).
/// O parametro C retorna o caminho encontrado
/// (vazio se  parametros invalidos ou nao existe caminho).
/// O parametro NA retorna o numero de nos em aberto ao termino do algoritmo A*
/// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
/// O parametro NF retorna o numero de nos em fechado ao termino do algoritmo A*
/// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
double Planejador::calculaCaminho(const IDPonto& id_origem,
                                  const IDPonto& id_destino,
                                  Caminho& C, int& NA, int& NF)
{
  // Zera o caminho resultado
  C.clear();

  try
  {
    // Mapa vazio
    if (empty()) throw 1;

    // Calcula o ponto que corresponde a id_origem.
    // Se nao existir, throw 4
    Ponto pt_orig = getPonto(id_origem);
    if (!pt_orig.valid()) throw 4;

    // Calcula o ponto que corresponde a id_destino.
    // Se nao existir, throw 5
    Ponto pt_dest = getPonto(id_destino);
    if (!pt_dest.valid()) throw 5;

    /* *****************************  /
    /  IMPLEMENTACAO DO ALGORITMO A*  /
    /  ***************************** */
    /* ***********  /
    /  FALTA FAZER  /
    /  *********** */

    Noh atual(id_origem, IDRota(), 0.0, haversine(pt_orig,pt_dest));

    // Inicializa os conjuntos de aberto e fechado
    list<Noh> aberto, fechado;

    do {
      //Lê e exclui o primeiro elemento do conjunto aberto
      atual = aberto.front();
      aberto.pop_front();

      //Inclui atual em fechado
      fechado.push_back(atual);

      //Expande se não é solução
      if (atual.id_pt != id_destino) {
        //Gera sucessores de atual
        auto itr_rotaalg = rotas.begin();
        do {
          //Busca rota-suc de atual
          itr_rotaalg = find(itr_rotaalg, rotas.end(), atual.id_rt);
          if (itr_rotaalg != rotas.end()) {
            //Gera noh sucessor
            Noh suc(itr_rotaalg->extremidade[1], IDRota(), atual.g + itr_rotaalg->comprimento, haversine(getPonto(atual.id), getPonto(itr_rotaalg->extremidade[1])));

            //Ponto do noh sucessor
             Ponto pt_suc = getPonto(suc.id_pt);

            //Assume que não existe Noh igual a "suc" nos containeres
            bool inedito = true;

            //Procura por "suc" em fechado
            auto itr_fechado = find(fechado.begin(), fechado.end(), suc.id_pt);
            if (itr_fechado != fechado.end()) {
                //Noh ja existe em fechado
                  inedito = false;
              }

            //Procura por "suc" em aberto
            auto itr_aberto = find(aberto.begin(), aberto.end(), suc.id_pt);
            if (itr_aberto != aberto.end()) {
              //Se o custo de "suc" em aberto for maior que o custo de "suc" atual
              if (itr_aberto->g > suc.g) {
                 //Exclui o antigo "suc" de aberto
                aberto.erase(itr_aberto);
              }
              else {
                //Noh ja existe em aberto
                inedito = false;
              }
            }
            if (inedito) {
            //Acha o primeiro noh de aberto com f() maior que o de "suc"
            auto big = aberto.begin();
            while (big != aberto.end() && big->f() < suc.f()) {
              big++;
            }

            //Insere "suc" em aberto
            aberto.insert(big, suc);
          }
          }
        } while (itr_rotaalg != rotas.end());

      
      }
    } while ((!aberto.empty() && atual.id_pt != id_destino));

    // O try tem que terminar retornando o comprimento calculado
    return -1.0;  // SUBSTITUA pelo return do valor correto
  }
  catch(int i)
  {
    cerr << "Erro " << i << " no calculo do caminho\n";
  }

  // Soh chega aqui se executou o catch, jah que o try termina sempre com return.
  // Caminho C permanece vazio.
  NA = NF = -1;
  return -1.0;
}
