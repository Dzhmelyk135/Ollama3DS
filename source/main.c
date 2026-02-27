/**
 * Ollama 3DS Homebrew Client
 * - Multi-lingua: EN, IT, FR, ES, DE, UK, JA
 * - Context di 5 messaggi
 * - Configurazione salvata su SD
 * - Crediti con L/R
 */

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>

// ── Percorso config su SD ────────────────────────────────────────────────────
#define CONFIG_DIR  "/3ds/ollama3ds"
#define CONFIG_PATH "/3ds/ollama3ds/config.ini"

// ── Valori di default ────────────────────────────────────────────────────────
#define DEFAULT_HOST   "192.168.1.100"
#define DEFAULT_PORT   "11434"
#define DEFAULT_MODEL  "gemma3"

// ── Limiti ───────────────────────────────────────────────────────────────────
#define MAX_HOST      64
#define MAX_PORT       8
#define MAX_MODEL     64
#define MAX_PROMPT   256
#define MAX_RESPONSE 4096

#define CTX_MAX_PAIRS  5
#define CTX_MSG_LEN  512

#define SOC_BUFFER_ALIGN  0x1000
#define SOC_BUFFER_SIZE   (256 * 1024)
#define HTTPC_SHMEM_SIZE  (32  * 1024)

#define BODY_MAX (CTX_MAX_PAIRS * 2 * (CTX_MSG_LEN * 2 + 64) + 256)

// ════════════════════════════════════════════════════════════════════════════
// I18N — Stringhe localizzate
// ════════════════════════════════════════════════════════════════════════════

typedef enum {
    LANG_EN = 0,
    LANG_IT,
    LANG_FR,
    LANG_ES,
    LANG_DE,
    LANG_UK,
    LANG_JA,
    LANG_COUNT
} LangID;

typedef struct {
    const char *name;

    const char *title;
    const char *lbl_model;
    const char *lbl_server;
    const char *lbl_memory;
    const char *btn_write;
    const char *btn_clear;
    const char *btn_config;
    const char *btn_credits;
    const char *btn_lang;
    const char *btn_exit;

    const char *init_net;
    const char *net_ok;
    const char *err_memalign_httpc;
    const char *err_memalign_soc;
    const char *err_socinit;
    const char *mem_cleared;
    const char *connecting;
    const char *no_response;
    const char *cancelled;

    const char *lbl_you;
    const char *lbl_error;
    const char *lbl_context;
    const char *lbl_no_msgs;

    const char *hint_main;
    const char *hint_result;
    const char *hint_context;

    const char *cfg_title;
    const char *cfg_host;
    const char *cfg_port;
    const char *cfg_model;
    const char *cfg_save;
    const char *cfg_back;
    const char *cfg_saved;
    const char *cfg_press_a;
    const char *cfg_nav;
    const char *cfg_exit;
    const char *kb_hint_host;
    const char *kb_hint_port;
    const char *kb_hint_model;
    const char *kb_hint_prompt;
    const char *kb_btn_cancel;
    const char *kb_btn_ok;
    const char *kb_btn_send;

    const char *cred_title;
    const char *cred_exit;
} Lang;

static const Lang langs[LANG_COUNT] = {

[LANG_EN] = {
    .name              = "English",
    .title             = "=== Ollama 3DS ===",
    .lbl_model         = "Model  :",
    .lbl_server        = "Server :",
    .lbl_memory        = "Memory :",
    .btn_write         = "[A]      Write message",
    .btn_clear         = "[Y]      Clear memory",
    .btn_config        = "[SELECT] Settings",
    .btn_credits       = "[L/R]    Credits",
    .btn_lang          = "[X]      Language",
    .btn_exit          = "[START]  Exit",
    .init_net          = "Initializing network...",
    .net_ok            = "Network OK!",
    .err_memalign_httpc= "[ERROR] memalign httpc failed",
    .err_memalign_soc  = "[ERROR] memalign soc failed",
    .err_socinit       = "[ERROR] socInit failed",
    .mem_cleared       = "Memory cleared!",
    .connecting        = "[Connecting to model...]",
    .no_response       = "[No response]",
    .cancelled         = "Cancelled.",
    .lbl_you           = "[You]",
    .lbl_error         = "[ERROR]",
    .lbl_context       = "=== Chat Context ===",
    .lbl_no_msgs       = "No messages in memory.",
    .hint_main         = "[A]Write [Y]Clear [L/R]Credits [START]Exit",
    .hint_result       = "[A]New [Y]Clear [SELECT]Settings [START]Exit",
    .hint_context      = "[Y] Clear  [SELECT] Settings",
    .cfg_title         = "=== SETTINGS ===",
    .cfg_host          = "Host (IP)",
    .cfg_port          = "Port",
    .cfg_model         = "Model",
    .cfg_save          = "Save to SD",
    .cfg_back          = "Back",
    .cfg_saved         = "Saved!",
    .cfg_press_a       = "Press A to continue...",
    .cfg_nav           = "[Up/Down] Navigate  [A] Edit",
    .cfg_exit          = "[B/SELECT] Back",
    .kb_hint_host      = "Ollama server IP",
    .kb_hint_port      = "Port (default: 11434)",
    .kb_hint_model     = "Model name (e.g. gemma3)",
    .kb_hint_prompt    = "Enter your message...",
    .kb_btn_cancel     = "Cancel",
    .kb_btn_ok         = "OK",
    .kb_btn_send       = "Send",
    .cred_title        = "=== CREDITS ===",
    .cred_exit         = "[B] Back",
},

[LANG_IT] = {
    .name              = "Italiano",
    .title             = "=== Ollama 3DS ===",
    .lbl_model         = "Modello:",
    .lbl_server        = "Server :",
    .lbl_memory        = "Memoria:",
    .btn_write         = "[A]      Scrivi messaggio",
    .btn_clear         = "[Y]      Azzera memoria",
    .btn_config        = "[SELECT] Impostazioni",
    .btn_credits       = "[L/R]    Crediti",
    .btn_lang          = "[X]      Lingua",
    .btn_exit          = "[START]  Esci",
    .init_net          = "Inizializzo la rete...",
    .net_ok            = "Rete OK!",
    .err_memalign_httpc= "[ERRORE] memalign httpc fallito",
    .err_memalign_soc  = "[ERRORE] memalign soc fallito",
    .err_socinit       = "[ERRORE] socInit fallito",
    .mem_cleared       = "Memoria azzerata!",
    .connecting        = "[Connessione al modello...]",
    .no_response       = "[Nessuna risposta]",
    .cancelled         = "Annullato.",
    .lbl_you           = "[Tu]",
    .lbl_error         = "[ERRORE]",
    .lbl_context       = "=== Chat Context ===",
    .lbl_no_msgs       = "Nessun messaggio in memoria.",
    .hint_main         = "[A]Scrivi [Y]Azzera [L/R]Crediti [START]Esci",
    .hint_result       = "[A]Nuovo [Y]Azzera [SELECT]Config [START]Esci",
    .hint_context      = "[Y] Azzera  [SELECT] Config",
    .cfg_title         = "=== IMPOSTAZIONI ===",
    .cfg_host          = "Host (IP)",
    .cfg_port          = "Porta",
    .cfg_model         = "Modello",
    .cfg_save          = "Salva su SD",
    .cfg_back          = "Torna",
    .cfg_saved         = "Salvato!",
    .cfg_press_a       = "Premi A per continuare...",
    .cfg_nav           = "[Su/Giu] Naviga  [A] Modifica",
    .cfg_exit          = "[B/SELECT] Esci",
    .kb_hint_host      = "IP server Ollama",
    .kb_hint_port      = "Porta (default: 11434)",
    .kb_hint_model     = "Nome modello (es. gemma3)",
    .kb_hint_prompt    = "Inserisci messaggio...",
    .kb_btn_cancel     = "Annulla",
    .kb_btn_ok         = "OK",
    .kb_btn_send       = "Invia",
    .cred_title        = "=== CREDITI ===",
    .cred_exit         = "[B] Indietro",
},

[LANG_FR] = {
    .name              = "Francais",
    .title             = "=== Ollama 3DS ===",
    .lbl_model         = "Modele :",
    .lbl_server        = "Serveur:",
    .lbl_memory        = "Memoire:",
    .btn_write         = "[A]      Ecrire un message",
    .btn_clear         = "[Y]      Effacer memoire",
    .btn_config        = "[SELECT] Parametres",
    .btn_credits       = "[L/R]    Credits",
    .btn_lang          = "[X]      Langue",
    .btn_exit          = "[START]  Quitter",
    .init_net          = "Initialisation reseau...",
    .net_ok            = "Reseau OK!",
    .err_memalign_httpc= "[ERREUR] memalign httpc echoue",
    .err_memalign_soc  = "[ERREUR] memalign soc echoue",
    .err_socinit       = "[ERREUR] socInit echoue",
    .mem_cleared       = "Memoire effacee!",
    .connecting        = "[Connexion au modele...]",
    .no_response       = "[Aucune reponse]",
    .cancelled         = "Annule.",
    .lbl_you           = "[Vous]",
    .lbl_error         = "[ERREUR]",
    .lbl_context       = "=== Contexte Chat ===",
    .lbl_no_msgs       = "Aucun message en memoire.",
    .hint_main         = "[A]Ecrire [Y]Effacer [L/R]Credits [START]Quitter",
    .hint_result       = "[A]Nouveau [Y]Effacer [SELECT]Config [START]Quitter",
    .hint_context      = "[Y] Effacer  [SELECT] Config",
    .cfg_title         = "=== PARAMETRES ===",
    .cfg_host          = "Hote (IP)",
    .cfg_port          = "Port",
    .cfg_model         = "Modele",
    .cfg_save          = "Sauvegarder",
    .cfg_back          = "Retour",
    .cfg_saved         = "Sauvegarde!",
    .cfg_press_a       = "Appuyez sur A...",
    .cfg_nav           = "[Haut/Bas] Naviguer  [A] Modifier",
    .cfg_exit          = "[B/SELECT] Retour",
    .kb_hint_host      = "IP du serveur Ollama",
    .kb_hint_port      = "Port (defaut: 11434)",
    .kb_hint_model     = "Nom du modele (ex. gemma3)",
    .kb_hint_prompt    = "Entrez votre message...",
    .kb_btn_cancel     = "Annuler",
    .kb_btn_ok         = "OK",
    .kb_btn_send       = "Envoyer",
    .cred_title        = "=== CREDITS ===",
    .cred_exit         = "[B] Retour",
},

[LANG_ES] = {
    .name              = "Espanol",
    .title             = "=== Ollama 3DS ===",
    .lbl_model         = "Modelo :",
    .lbl_server        = "Servidor:",
    .lbl_memory        = "Memoria:",
    .btn_write         = "[A]      Escribir mensaje",
    .btn_clear         = "[Y]      Borrar memoria",
    .btn_config        = "[SELECT] Ajustes",
    .btn_credits       = "[L/R]    Creditos",
    .btn_lang          = "[X]      Idioma",
    .btn_exit          = "[START]  Salir",
    .init_net          = "Iniciando red...",
    .net_ok            = "Red OK!",
    .err_memalign_httpc= "[ERROR] memalign httpc fallido",
    .err_memalign_soc  = "[ERROR] memalign soc fallido",
    .err_socinit       = "[ERROR] socInit fallido",
    .mem_cleared       = "Memoria borrada!",
    .connecting        = "[Conectando al modelo...]",
    .no_response       = "[Sin respuesta]",
    .cancelled         = "Cancelado.",
    .lbl_you           = "[Tu]",
    .lbl_error         = "[ERROR]",
    .lbl_context       = "=== Contexto Chat ===",
    .lbl_no_msgs       = "Sin mensajes en memoria.",
    .hint_main         = "[A]Escribir [Y]Borrar [L/R]Creditos [START]Salir",
    .hint_result       = "[A]Nuevo [Y]Borrar [SELECT]Config [START]Salir",
    .hint_context      = "[Y] Borrar  [SELECT] Config",
    .cfg_title         = "=== AJUSTES ===",
    .cfg_host          = "Host (IP)",
    .cfg_port          = "Puerto",
    .cfg_model         = "Modelo",
    .cfg_save          = "Guardar en SD",
    .cfg_back          = "Volver",
    .cfg_saved         = "Guardado!",
    .cfg_press_a       = "Pulsa A para continuar...",
    .cfg_nav           = "[Arriba/Abajo] Navegar  [A] Editar",
    .cfg_exit          = "[B/SELECT] Volver",
    .kb_hint_host      = "IP del servidor Ollama",
    .kb_hint_port      = "Puerto (defecto: 11434)",
    .kb_hint_model     = "Nombre del modelo (ej. gemma3)",
    .kb_hint_prompt    = "Escribe tu mensaje...",
    .kb_btn_cancel     = "Cancelar",
    .kb_btn_ok         = "OK",
    .kb_btn_send       = "Enviar",
    .cred_title        = "=== CREDITOS ===",
    .cred_exit         = "[B] Volver",
},

[LANG_DE] = {
    .name              = "Deutsch",
    .title             = "=== Ollama 3DS ===",
    .lbl_model         = "Modell :",
    .lbl_server        = "Server :",
    .lbl_memory        = "Speicher:",
    .btn_write         = "[A]      Nachricht schreiben",
    .btn_clear         = "[Y]      Speicher leeren",
    .btn_config        = "[SELECT] Einstellungen",
    .btn_credits       = "[L/R]    Credits",
    .btn_lang          = "[X]      Sprache",
    .btn_exit          = "[START]  Beenden",
    .init_net          = "Netzwerk wird initialisiert...",
    .net_ok            = "Netzwerk OK!",
    .err_memalign_httpc= "[FEHLER] memalign httpc fehlgeschlagen",
    .err_memalign_soc  = "[FEHLER] memalign soc fehlgeschlagen",
    .err_socinit       = "[FEHLER] socInit fehlgeschlagen",
    .mem_cleared       = "Speicher geleert!",
    .connecting        = "[Verbinde mit Modell...]",
    .no_response       = "[Keine Antwort]",
    .cancelled         = "Abgebrochen.",
    .lbl_you           = "[Du]",
    .lbl_error         = "[FEHLER]",
    .lbl_context       = "=== Chat-Kontext ===",
    .lbl_no_msgs       = "Keine Nachrichten im Speicher.",
    .hint_main         = "[A]Schreiben [Y]Leeren [L/R]Credits [START]Beenden",
    .hint_result       = "[A]Neu [Y]Leeren [SELECT]Config [START]Beenden",
    .hint_context      = "[Y] Leeren  [SELECT] Config",
    .cfg_title         = "=== EINSTELLUNGEN ===",
    .cfg_host          = "Host (IP)",
    .cfg_port          = "Port",
    .cfg_model         = "Modell",
    .cfg_save          = "Auf SD speichern",
    .cfg_back          = "Zuruck",
    .cfg_saved         = "Gespeichert!",
    .cfg_press_a       = "A druecken um fortzufahren...",
    .cfg_nav           = "[Hoch/Runter] Navigieren  [A] Bearbeiten",
    .cfg_exit          = "[B/SELECT] Zuruck",
    .kb_hint_host      = "IP des Ollama-Servers",
    .kb_hint_port      = "Port (Standard: 11434)",
    .kb_hint_model     = "Modellname (z.B. gemma3)",
    .kb_hint_prompt    = "Nachricht eingeben...",
    .kb_btn_cancel     = "Abbrechen",
    .kb_btn_ok         = "OK",
    .kb_btn_send       = "Senden",
    .cred_title        = "=== CREDITS ===",
    .cred_exit         = "[B] Zuruck",
},

[LANG_UK] = {
    .name              = "Ukrayinska",
    .title             = "=== Ollama 3DS ===",
    .lbl_model         = "Model  :",
    .lbl_server        = "Server :",
    .lbl_memory        = "Pamyat :",
    .btn_write         = "[A]      Napysaty povidomlennya",
    .btn_clear         = "[Y]      Ochystyty pamyat",
    .btn_config        = "[SELECT] Nalashtuvannya",
    .btn_credits       = "[L/R]    Kredyty",
    .btn_lang          = "[X]      Mova",
    .btn_exit          = "[START]  Vykhid",
    .init_net          = "Initsializatsiya merezhi...",
    .net_ok            = "Merezha OK!",
    .err_memalign_httpc= "[POMYLKA] memalign httpc ne vdavsya",
    .err_memalign_soc  = "[POMYLKA] memalign soc ne vdavsya",
    .err_socinit       = "[POMYLKA] socInit ne vdavsya",
    .mem_cleared       = "Pamyat ochyshcheno!",
    .connecting        = "[Pidklyuchennya do modeli...]",
    .no_response       = "[Vidpovidi nemaye]",
    .cancelled         = "Skasovano.",
    .lbl_you           = "[Vy]",
    .lbl_error         = "[POMYLKA]",
    .lbl_context       = "=== Kontekst chatu ===",
    .lbl_no_msgs       = "Povidomlen u pamyati nemaye.",
    .hint_main         = "[A]Pysaty [Y]Ochystyty [L/R]Kredyty [START]Vykhid",
    .hint_result       = "[A]Nove [Y]Ochystyty [SELECT]Config [START]Vykhid",
    .hint_context      = "[Y] Ochystyty  [SELECT] Config",
    .cfg_title         = "=== NALASHTUVANNYA ===",
    .cfg_host          = "Khost (IP)",
    .cfg_port          = "Port",
    .cfg_model         = "Model",
    .cfg_save          = "Zberehty na SD",
    .cfg_back          = "Nazad",
    .cfg_saved         = "Zberezeno!",
    .cfg_press_a       = "Natisnit A shchob prodovzhyty...",
    .cfg_nav           = "[Vhoru/Vnyz] Navigatsiya  [A] Zminyty",
    .cfg_exit          = "[B/SELECT] Nazad",
    .kb_hint_host      = "IP servera Ollama",
    .kb_hint_port      = "Port (za zamovchuvannyam: 11434)",
    .kb_hint_model     = "Nazva modeli (np. gemma3)",
    .kb_hint_prompt    = "Vvedit povidomlennya...",
    .kb_btn_cancel     = "Skasuvaty",
    .kb_btn_ok         = "OK",
    .kb_btn_send       = "Nadisaty",
    .cred_title        = "=== KREDYTY ===",
    .cred_exit         = "[B] Nazad",
},

[LANG_JA] = {
    .name              = "Nihongo",
    .title             = "=== Ollama 3DS ===",
    .lbl_model         = "Moderu:",
    .lbl_server        = "Saba  :",
    .lbl_memory        = "Kioku :",
    .btn_write         = "[A]      Messeji wo kaku",
    .btn_clear         = "[Y]      Kioku wo keshi",
    .btn_config        = "[SELECT] Settei",
    .btn_credits       = "[L/R]    Kurejitto",
    .btn_lang          = "[X]      Gengo",
    .btn_exit          = "[START]  Owari",
    .init_net          = "Nettowaku shoki-ka chuu...",
    .net_ok            = "Nettowaku OK!",
    .err_memalign_httpc= "[ERROR] memalign httpc shippai",
    .err_memalign_soc  = "[ERROR] memalign soc shippai",
    .err_socinit       = "[ERROR] socInit shippai",
    .mem_cleared       = "Kioku wo keshimashita!",
    .connecting        = "[Moderu ni setsuzoku chuu...]",
    .no_response       = "[Outou nashi]",
    .cancelled         = "Kyanseru shimashita.",
    .lbl_you           = "[Anata]",
    .lbl_error         = "[ERROR]",
    .lbl_context       = "=== Chatto Kontekusuto ===",
    .lbl_no_msgs       = "Kioku ni messeji ga arimasen.",
    .hint_main         = "[A]Kaku [Y]Kesu [L/R]Kurejitto [START]Owari",
    .hint_result       = "[A]Atarashii [Y]Kesu [SELECT]Settei [START]Owari",
    .hint_context      = "[Y] Kesu  [SELECT] Settei",
    .cfg_title         = "=== SETTEI ===",
    .cfg_host          = "Hosuto (IP)",
    .cfg_port          = "Pooto",
    .cfg_model         = "Moderu",
    .cfg_save          = "SD ni hozon",
    .cfg_back          = "Modoru",
    .cfg_saved         = "Hozon shimashita!",
    .cfg_press_a       = "A wo oshite kudasai...",
    .cfg_nav           = "[Ue/Shita] Idou  [A] Henshu",
    .cfg_exit          = "[B/SELECT] Modoru",
    .kb_hint_host      = "Ollama saaba no IP",
    .kb_hint_port      = "Pooto (default: 11434)",
    .kb_hint_model     = "Moderu mei (rei: gemma3)",
    .kb_hint_prompt    = "Messeji wo nyuuryoku...",
    .kb_btn_cancel     = "Kyanseru",
    .kb_btn_ok         = "OK",
    .kb_btn_send       = "Okuru",
    .cred_title        = "=== KUREJITTO ===",
    .cred_exit         = "[B] Modoru",
},

}; // fine langs[]

static LangID cur_lang = LANG_EN;
#define T (langs[cur_lang])

// ════════════════════════════════════════════════════════════════════════════
// Strutture dati
// ════════════════════════════════════════════════════════════════════════════

typedef struct {
    char host[MAX_HOST];
    char port[MAX_PORT];
    char model[MAX_MODEL];
} OllamaConfig;

typedef struct {
    char role[16];
    char content[CTX_MSG_LEN];
} ChatMessage;

typedef struct {
    ChatMessage msgs[CTX_MAX_PAIRS * 2];
    int         count;
} ChatHistory;

static u32         *soc_buffer  = NULL;
static u32         *httpc_shmem = NULL;
static char         response_buf[MAX_RESPONSE];
static char         prompt_buf[MAX_PROMPT];
static OllamaConfig cfg;
static ChatHistory  history;

static char s_body[BODY_MAX];
static char s_raw[MAX_RESPONSE];
static char s_esc[CTX_MSG_LEN * 2 + 4];
static u8   s_buf[1024];

// ════════════════════════════════════════════════════════════════════════════
// HISTORY
// ════════════════════════════════════════════════════════════════════════════

static void history_clear(void)
{
    memset(&history, 0, sizeof(history));
    history.count = 0;
}

static void history_add(const char *role, const char *content)
{
    int max_msgs = CTX_MAX_PAIRS * 2;
    if (history.count >= max_msgs) {
        for (int i = 0; i < max_msgs - 2; i++)
            history.msgs[i] = history.msgs[i + 2];
        history.count = max_msgs - 2;
    }
    ChatMessage *m = &history.msgs[history.count];
    strncpy(m->role,    role,    sizeof(m->role)    - 1); m->role[sizeof(m->role)-1]       = '\0';
    strncpy(m->content, content, sizeof(m->content) - 1); m->content[sizeof(m->content)-1] = '\0';
    history.count++;
}

// ════════════════════════════════════════════════════════════════════════════
// CONFIG
// ════════════════════════════════════════════════════════════════════════════

static void config_defaults(void)
{
    strncpy(cfg.host,  DEFAULT_HOST,  sizeof(cfg.host)  - 1); cfg.host[sizeof(cfg.host)-1]   = '\0';
    strncpy(cfg.port,  DEFAULT_PORT,  sizeof(cfg.port)  - 1); cfg.port[sizeof(cfg.port)-1]   = '\0';
    strncpy(cfg.model, DEFAULT_MODEL, sizeof(cfg.model) - 1); cfg.model[sizeof(cfg.model)-1] = '\0';
}

static void config_load(void)
{
    config_defaults();
    cur_lang = LANG_EN;
    FILE *f = fopen(CONFIG_PATH, "r");
    if (!f) return;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = line, *val = eq + 1;
        if (strcmp(key,"host")  == 0){ strncpy(cfg.host,  val, sizeof(cfg.host) -1);  cfg.host[sizeof(cfg.host)-1]  ='\0'; }
        if (strcmp(key,"port")  == 0){ strncpy(cfg.port,  val, sizeof(cfg.port) -1);  cfg.port[sizeof(cfg.port)-1]  ='\0'; }
        if (strcmp(key,"model") == 0){ strncpy(cfg.model, val, sizeof(cfg.model)-1);  cfg.model[sizeof(cfg.model)-1]='\0'; }
        if (strcmp(key,"lang")  == 0){ int l = atoi(val); if (l >= 0 && l < LANG_COUNT) cur_lang = (LangID)l; }
    }
    fclose(f);
}

static void config_save(void)
{
    mkdir(CONFIG_DIR, 0777);
    FILE *f = fopen(CONFIG_PATH, "w");
    if (!f) return;
    fprintf(f, "host=%s\nport=%s\nmodel=%s\nlang=%d\n",
            cfg.host, cfg.port, cfg.model, (int)cur_lang);
    fclose(f);
}

// ════════════════════════════════════════════════════════════════════════════
// JSON helpers
// ════════════════════════════════════════════════════════════════════════════

static size_t json_escape_to(const char *src, char *dst, size_t dst_len)
{
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 3 < dst_len; i++) {
        unsigned char c = (unsigned char)src[i];
        switch (c) {
            case '"':  dst[j++]='\\'; dst[j++]='"';  break;
            case '\\': dst[j++]='\\'; dst[j++]='\\'; break;
            case '\n': dst[j++]='\\'; dst[j++]='n';  break;
            case '\r': break;
            case '\t': dst[j++]='\\'; dst[j++]='t';  break;
            default:   if (c >= 0x20) dst[j++]=(char)c; break;
        }
    }
    dst[j] = '\0';
    return j;
}

static void build_chat_body(char *out, size_t out_len)
{
    size_t pos = 0;
    #define APPEND(str) do { size_t _l=strlen(str); if(pos+_l+1<out_len){memcpy(out+pos,str,_l);pos+=_l;} } while(0)
    APPEND("{\"model\":\""); APPEND(cfg.model); APPEND("\",\"stream\":true,\"messages\":[");
    for (int i = 0; i < history.count; i++) {
        if (i > 0) APPEND(",");
        APPEND("{\"role\":\""); APPEND(history.msgs[i].role); APPEND("\",\"content\":\"");
        json_escape_to(history.msgs[i].content, s_esc, sizeof(s_esc));
        APPEND(s_esc);
        APPEND("\"}");
    }
    APPEND("]}");
    out[pos] = '\0';
    #undef APPEND
}

static void extract_chat_response(const char *raw, char *out, size_t out_len)
{
    const char *ptr = raw;
    size_t total = 0;
    out[0] = '\0';
    while (*ptr && total < out_len - 1) {
        const char *key = strstr(ptr, "\"content\":\"");
        if (!key) break;
        key += strlen("\"content\":\"");
        while (*key && total < out_len - 1) {
            if (*key == '\\' && *(key+1)) {
                key++;
                switch (*key) {
                    case 'n':  out[total++] = '\n'; break;
                    case 'r':                       break;
                    case 't':  out[total++] = '\t'; break;
                    case '"':  out[total++] = '"';  break;
                    case '\\': out[total++] = '\\'; break;
                    default:   out[total++] = *key; break;
                }
                key++;
            } else if (*key == '"') { key++; break; }
            else { out[total++] = *key++; }
        }
        ptr = key;
    }
    out[total] = '\0';
}

// ════════════════════════════════════════════════════════════════════════════
// OLLAMA — HTTP POST /api/chat
// ════════════════════════════════════════════════════════════════════════════

static int ollama_chat(char *out, size_t out_len)
{
    static char url[MAX_HOST + MAX_PORT + 32];
    snprintf(url, sizeof(url), "http://%s:%s/api/chat", cfg.host, cfg.port);
    build_chat_body(s_body, sizeof(s_body));

    httpcContext ctx;
    Result rc = httpcOpenContext(&ctx, HTTPC_METHOD_POST, url, 0);
    if (R_FAILED(rc)) { snprintf(out, out_len, "[OpenContext:\n0x%08lX]", rc); return -1; }

    httpcSetSSLOpt(&ctx, SSLCOPT_DisableVerify);
    httpcSetKeepAlive(&ctx, HTTPC_KEEPALIVE_ENABLED);
    httpcAddRequestHeaderField(&ctx, "User-Agent",   "Ollama3DS/1.0");
    httpcAddRequestHeaderField(&ctx, "Content-Type", "application/json");
    httpcAddRequestHeaderField(&ctx, "Connection",   "close");

    rc = httpcAddPostDataRaw(&ctx, (u32*)s_body, (u32)strlen(s_body));
    if (R_FAILED(rc)) { httpcCloseContext(&ctx); snprintf(out, out_len, "[PostDataRaw:\n0x%08lX]", rc); return -1; }

    rc = httpcBeginRequest(&ctx);
    if (R_FAILED(rc)) { httpcCloseContext(&ctx); snprintf(out, out_len, "[BeginRequest:\n0x%08lX]", rc); return -1; }

    u32 status_code = 0;
    rc = httpcGetResponseStatusCode(&ctx, &status_code);
    if (R_FAILED(rc))       { httpcCloseContext(&ctx); snprintf(out, out_len, "[GetStatus:\n0x%08lX]", rc); return -1; }
    if (status_code != 200) { httpcCloseContext(&ctx); snprintf(out, out_len, "[HTTP %lu]", (unsigned long)status_code); return -1; }

    u32 raw_size = 0;
    memset(s_raw, 0, sizeof(s_raw));
    do {
        memset(s_buf, 0, sizeof(s_buf));
        rc = httpcReceiveData(&ctx, s_buf, sizeof(s_buf) - 1);
        size_t chunk = strlen((char*)s_buf);
        if (raw_size + chunk < sizeof(s_raw) - 1) { memcpy(s_raw + raw_size, s_buf, chunk); raw_size += chunk; }
    } while (rc == (Result)HTTPC_RESULTCODE_DOWNLOADPENDING);

    httpcCloseContext(&ctx);
    extract_chat_response(s_raw, out, out_len);
    if (out[0] == '\0') strncpy(out, T.no_response, out_len - 1);
    return 0;
}

// ════════════════════════════════════════════════════════════════════════════
// UI — Selettore lingua (tasto X)
// ════════════════════════════════════════════════════════════════════════════

static void draw_lang_menu(PrintConsole *con, int sel)
{
    consoleSelect(con); consoleClear();
    printf("\x1b[0;0H");
    printf(" === LANGUAGE / LINGUA ===\n\n");
    for (int i = 0; i < LANG_COUNT; i++) {
        if (i == sel) printf(" \x1b[32m> %s\x1b[0m\n", langs[i].name);
        else          printf("   %s\n", langs[i].name);
    }
    printf("\n \x1b[36m[Su/Giu] Scegli  [A] Conferma\x1b[0m\n");
    printf(" \x1b[36m[B] Annulla\x1b[0m\n");
}

static void open_lang_menu(PrintConsole *top_con, PrintConsole *bottom_con)
{
    int sel = (int)cur_lang;
    draw_lang_menu(bottom_con, sel);
    while (aptMainLoop()) {
        gspWaitForVBlank(); hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_B) break;
        if (kDown & KEY_UP)   { sel = (sel > 0) ? sel-1 : LANG_COUNT-1; draw_lang_menu(bottom_con, sel); }
        if (kDown & KEY_DOWN) { sel = (sel < LANG_COUNT-1) ? sel+1 : 0; draw_lang_menu(bottom_con, sel); }
        if (kDown & KEY_A)    { cur_lang = (LangID)sel; config_save(); break; }
        gfxFlushBuffers(); gfxSwapBuffers();
    }
    consoleSelect(bottom_con); consoleClear();
    (void)top_con;
}

// ════════════════════════════════════════════════════════════════════════════
// UI — Schermata Crediti (tasto L o R)
// ════════════════════════════════════════════════════════════════════════════

static void show_credits(PrintConsole *top_con, PrintConsole *bottom_con)
{
    consoleSelect(top_con); consoleClear();
    printf("\x1b[0;0H");
    printf("\x1b[33m%s\x1b[0m\n\n", T.cred_title);
    printf("\x1b[32m-- Developer --\x1b[0m\n");
    printf(" Dzhmelyk135\n");
    printf(" github.com/Dzhmelyk135/Ollama3DS\n\n");
    printf("\x1b[32m-- Tools & Libraries --\x1b[0m\n");
    printf(" devkitPro / devkitARM\n");
    printf("   toolchain for 3DS homebrew\n\n");
    printf(" libctru\n");
    printf("   3DS system library (httpc,\n");
    printf("   swkbd, gfx, soc...)\n\n");
    printf(" Ollama\n");
    printf("   Local LLM inference server\n");
    printf("   ollama.com\n\n");
    printf(" GitHub Copilot\n");
    printf("   AI coding assistant\n\n");
    printf("\x1b[32m-- Tested Models --\x1b[0m\n");
    printf(" gemma3, mistral, llama3.2,\n");
    printf(" phi3:mini\n\n");
    printf("\x1b[36mv1.0  2025  Nintendo 3DS\x1b[0m\n");

    consoleSelect(bottom_con); consoleClear();
    printf("\x1b[0;0H\n\n");
    printf(" \x1b[33mOllama 3DS\x1b[0m\n\n");
    printf(" Run local LLMs on your 3DS\n");
    printf(" via Ollama HTTP API.\n\n");
    printf(" Requires Wi-Fi + Ollama\n");
    printf(" running on your PC/server.\n\n");
    printf(" \x1b[36m%s\x1b[0m\n", T.cred_exit);

    // ← FIX: esce con B (coerente con cred_exit "[B] Back")
    while (aptMainLoop()) {
        gspWaitForVBlank(); hidScanInput();
        if (hidKeysDown() & KEY_B) break;
        gfxFlushBuffers(); gfxSwapBuffers();
    }
}

// ════════════════════════════════════════════════════════════════════════════
// UI — Tastiera
// ════════════════════════════════════════════════════════════════════════════

static bool kb_ask(const char *hint, const char *initial,
                   char *out, size_t out_len, bool digits_only)
{
    SwkbdState swkbd;
    swkbdInit(&swkbd, digits_only ? SWKBD_TYPE_NUMPAD : SWKBD_TYPE_NORMAL, 2, out_len-1);
    swkbdSetHintText(&swkbd, hint);
    swkbdSetInitialText(&swkbd, initial);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT,  T.kb_btn_cancel, false);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, T.kb_btn_ok,     true);
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    char tmp[128] = {0};
    SwkbdButton btn = swkbdInputText(&swkbd, tmp, sizeof(tmp));
    if (btn == SWKBD_BUTTON_RIGHT || btn == SWKBD_BUTTON_CONFIRM) {
        strncpy(out, tmp, out_len-1); out[out_len-1]='\0'; return true;
    }
    return false;
}

static bool read_prompt(char *out, size_t max_len)
{
    SwkbdState swkbd;
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, max_len-1);
    swkbdSetHintText(&swkbd, T.kb_hint_prompt);
    swkbdSetButton(&swkbd, SWKBD_BUTTON_CONFIRM, T.kb_btn_send, true);
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    return swkbdInputText(&swkbd, out, max_len) == SWKBD_BUTTON_CONFIRM;
}

// ════════════════════════════════════════════════════════════════════════════
// UI — Menu configurazione
// ════════════════════════════════════════════════════════════════════════════

typedef enum { MENU_HOST=0, MENU_PORT, MENU_MODEL, MENU_SAVE, MENU_BACK, MENU_COUNT } MenuItem;

static void draw_config_menu(PrintConsole *con, MenuItem sel)
{
    consoleSelect(con); consoleClear();
    printf("\x1b[0;0H %s\n\n", T.cfg_title);
    const char *labels[MENU_COUNT];
    labels[MENU_HOST]  = T.cfg_host;
    labels[MENU_PORT]  = T.cfg_port;
    labels[MENU_MODEL] = T.cfg_model;
    labels[MENU_SAVE]  = T.cfg_save;
    labels[MENU_BACK]  = T.cfg_back;
    for (int i = 0; i < MENU_COUNT; i++) {
        if (i == (int)sel) printf("\x1b[32m> %s\x1b[0m\n", labels[i]);
        else               printf("  %s\n", labels[i]);
        if (i == MENU_HOST)  printf("  \x1b[33m%s\x1b[0m\n", cfg.host);
        if (i == MENU_PORT)  printf("  \x1b[33m%s\x1b[0m\n", cfg.port);
        if (i == MENU_MODEL) printf("  \x1b[33m%s\x1b[0m\n", cfg.model);
    }
    printf("\n\x1b[36m%s\x1b[0m\n", T.cfg_nav);
    printf("\x1b[36m%s\x1b[0m\n",   T.cfg_exit);
}

static void open_config_menu(PrintConsole *top_con, PrintConsole *bottom_con)
{
    MenuItem sel = MENU_HOST;
    draw_config_menu(bottom_con, sel);
    while (aptMainLoop()) {
        gspWaitForVBlank(); hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & (KEY_B | KEY_SELECT)) break;
        if (kDown & KEY_UP)   { sel = (sel>0) ? sel-1 : MENU_COUNT-1; draw_config_menu(bottom_con, sel); }
        if (kDown & KEY_DOWN) { sel = (sel<MENU_COUNT-1) ? sel+1 : 0; draw_config_menu(bottom_con, sel); }
        if (kDown & KEY_A) {
            bool changed = false;
            switch(sel) {
                case MENU_HOST:  changed=kb_ask(T.kb_hint_host,  cfg.host,  cfg.host,  MAX_HOST,  false); break;
                case MENU_PORT:  changed=kb_ask(T.kb_hint_port,  cfg.port,  cfg.port,  MAX_PORT,  true);  break;
                case MENU_MODEL: changed=kb_ask(T.kb_hint_model, cfg.model, cfg.model, MAX_MODEL, false); break;
                case MENU_SAVE:
                    config_save();
                    consoleSelect(bottom_con); consoleClear();
                    printf("\n\n  \x1b[32m%s\x1b[0m\n  %s\n\n  %s\n",
                           T.cfg_saved, CONFIG_PATH, T.cfg_press_a);
                    while(aptMainLoop()){
                        gspWaitForVBlank(); hidScanInput();
                        if(hidKeysDown()&KEY_A) break;
                        gfxFlushBuffers(); gfxSwapBuffers();
                    }
                    break;
                case MENU_BACK: goto exit_cfg;
                default: break;
            }
            (void)changed;
            draw_config_menu(bottom_con, sel);
        }
        gfxFlushBuffers(); gfxSwapBuffers();
    }
exit_cfg:;
    (void)top_con;
}

// ════════════════════════════════════════════════════════════════════════════
// UI — Schermi principali
// ════════════════════════════════════════════════════════════════════════════

static void print_wrapped(const char *text, int cols)
{
    int col = 0;
    for (const char *p = text; *p; p++) {
        if (*p == '\n') { putchar('\n'); col=0; continue; }
        if (col >= cols) { putchar('\n'); col=0; }
        putchar(*p); col++;
    }
    putchar('\n');
}

static void draw_main_screen(PrintConsole *con)
{
    consoleSelect(con); consoleClear();
    printf("\x1b[0;0H");
    printf("%s\n", T.title);
    printf("%s \x1b[33m%s\x1b[0m\n",     T.lbl_model,  cfg.model);
    printf("%s \x1b[33m%s:%s\x1b[0m\n",  T.lbl_server, cfg.host, cfg.port);
    printf("%s \x1b[36m%d/%d\x1b[0m\n\n",T.lbl_memory, history.count, CTX_MAX_PAIRS*2);
    printf("%s\n", T.btn_write);
    printf("%s\n", T.btn_clear);
    printf("%s\n", T.btn_config);
    printf("%s\n", T.btn_credits);
    printf("%s: \x1b[35m%s\x1b[0m\n", T.btn_lang, langs[cur_lang].name);
    printf("%s\n", T.btn_exit);
}

static void draw_bottom_info(PrintConsole *con)
{
    consoleSelect(con); consoleClear();
    printf("\x1b[0;0H");
    printf(" %s\n\n", T.lbl_context);
    if (history.count == 0) {
        printf(" \x1b[90m%s\x1b[0m\n", T.lbl_no_msgs);
    } else {
        int start = (history.count > 6) ? history.count-6 : 0;
        for (int i = start; i < history.count; i++) {
            bool is_user = (strcmp(history.msgs[i].role, "user") == 0);
            printf(" %s ", is_user ? "\x1b[33mYou:\x1b[0m" : "\x1b[32m AI:\x1b[0m");
            char preview[29] = {0};
            strncpy(preview, history.msgs[i].content, 28);
            for (int k=0; k<28; k++) if(preview[k]=='\n') preview[k]=' ';
            printf("%s%s\n", preview, strlen(history.msgs[i].content)>28?"...":"");
        }
    }
    printf("\n \x1b[36m%s\x1b[0m\n", T.hint_context);
}

// ════════════════════════════════════════════════════════════════════════════
// MAIN
// ════════════════════════════════════════════════════════════════════════════

int main(void)
{
    gfxInitDefault();
    PrintConsole top_con, bottom_con;
    consoleInit(GFX_TOP,    &top_con);
    consoleInit(GFX_BOTTOM, &bottom_con);

    history_clear();
    config_load();

    consoleSelect(&top_con);
    printf("%s\n%s\n", T.title, T.init_net);

    acInit();

    httpc_shmem = (u32*)memalign(SOC_BUFFER_ALIGN, HTTPC_SHMEM_SIZE);
    if (!httpc_shmem) { printf("%s\n", T.err_memalign_httpc); goto cleanup; }
    httpcInit(HTTPC_SHMEM_SIZE);

    soc_buffer = (u32*)memalign(SOC_BUFFER_ALIGN, SOC_BUFFER_SIZE);
    if (!soc_buffer) { printf("%s\n", T.err_memalign_soc); goto cleanup; }
    if (R_FAILED(socInit(soc_buffer, SOC_BUFFER_SIZE))) { printf("%s\n", T.err_socinit); goto cleanup; }

    draw_main_screen(&top_con);
    draw_bottom_info(&bottom_con);

    while (aptMainLoop()) {
        gspWaitForVBlank(); hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        // ── Azzera memoria ───────────────────────────────────────────────────
        if (kDown & KEY_Y) {
            history_clear();
            draw_main_screen(&top_con);
            draw_bottom_info(&bottom_con);
            consoleSelect(&top_con);
            printf("\n\x1b[32m%s\x1b[0m\n", T.mem_cleared);
        }

        // ── Crediti: L oppure R ──────────────────────────────────────────────
        // ← FIX: era KEY_B, ora correttamente KEY_L | KEY_R
        if (kDown & (KEY_L | KEY_R)) {
            show_credits(&top_con, &bottom_con);
            draw_main_screen(&top_con);
            draw_bottom_info(&bottom_con);
        }

        // ── Selettore lingua: X ──────────────────────────────────────────────
        if (kDown & KEY_X) {
            open_lang_menu(&top_con, &bottom_con);
            draw_main_screen(&top_con);
            draw_bottom_info(&bottom_con);
        }

        // ── Configurazione: SELECT ───────────────────────────────────────────
        if (kDown & KEY_SELECT) {
            open_config_menu(&top_con, &bottom_con);
            draw_main_screen(&top_con);
            draw_bottom_info(&bottom_con);
        }

        // ── Nuovo messaggio: A ───────────────────────────────────────────────
        if (kDown & KEY_A) {
            memset(prompt_buf, 0, sizeof(prompt_buf));
            bool ok = read_prompt(prompt_buf, sizeof(prompt_buf));
            if (!ok || prompt_buf[0] == '\0') {
                draw_main_screen(&top_con);
                continue;
            }

            history_add("user", prompt_buf);

            consoleSelect(&top_con); consoleClear();
            printf("\x1b[0;0H%s\n\n", T.title);
            printf("\x1b[33m%s\x1b[0m\n", T.lbl_you);
            print_wrapped(prompt_buf, 49);
            printf("\n\x1b[36m%s\x1b[0m\n", T.connecting);
            gfxFlushBuffers(); gfxSwapBuffers();

            memset(response_buf, 0, sizeof(response_buf));
            int res = ollama_chat(response_buf, sizeof(response_buf));

            if (res == 0) {
                history_add("assistant", response_buf);
            } else {
                if (history.count > 0) history.count--;
            }

            consoleSelect(&top_con); consoleClear();
            printf("\x1b[0;0H%s\n\n", T.title);
            printf("\x1b[33m%s\x1b[0m\n", T.lbl_you);
            print_wrapped(prompt_buf, 49);

            if (res == 0) {
                printf("\n\x1b[32m[%s]\x1b[0m\n", cfg.model);
                print_wrapped(response_buf, 49);
            } else {
                printf("\n\x1b[31m%s\x1b[0m\n", T.lbl_error);
                print_wrapped(response_buf, 49);
            }

            printf("\n\x1b[36m%s\x1b[0m\n", T.hint_result);
            draw_bottom_info(&bottom_con);
            consoleSelect(&top_con);
        }

        gfxFlushBuffers(); gfxSwapBuffers();
    }

cleanup:
    if (soc_buffer)  { socExit(); free(soc_buffer); }
    httpcExit();
    if (httpc_shmem) free(httpc_shmem);
    acExit();
    gfxExit();
    return 0;
}