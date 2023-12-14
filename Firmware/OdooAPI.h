#include <HTTPClient.h>
#include <ArduinoJson.h>

const String url = "http://<ip>:<port>"; // Set your odoo url
const String db = "example"; //Set You Odoo DB Name
const String login = "odoo@example.com"; //Set your email
const String password = "mySuperStrongPassword"; //Set your password

// Structure to hold the results of the authentication.
struct AuthResult {
    String session_id;
    int uid;
};

String extractSessionId(const String &cookieHeader) {
    String prefix = "session_id=";
    int startIdx = cookieHeader.indexOf(prefix);
    
    if (startIdx == -1) {
        return "";
    }
    
    startIdx += prefix.length();  // Move to the start of the actual session ID value
    int endIdx = cookieHeader.indexOf(';', startIdx);  // Find the end of the session ID value

    if (endIdx == -1) {
        endIdx = cookieHeader.length();  // If there's no semicolon, take the rest of the string
    }

    return cookieHeader.substring(startIdx, endIdx);
}

DynamicJsonDocument trigger_method(String url, String session_id, int uid, int method_id) {
    HTTPClient http;

    // Construct the endpoint
    String endpoint = url + "/web/dataset/call_button";
    String payload;

    // Construct the JSON payload.
    const size_t capacity = JSON_OBJECT_SIZE(5) + 
                            JSON_OBJECT_SIZE(4) + 
                            JSON_OBJECT_SIZE(5) + 
                            JSON_OBJECT_SIZE(5);
    DynamicJsonDocument doc(capacity);

    doc["id"] = 15;
    doc["jsonrpc"] = "2.0";
    doc["method"] = "call";
    doc["params"]["args"][0] = method_id;
    doc["params"]["kwargs"]["context"]["search_default_all"] = 1;
    doc["params"]["kwargs"]["context"]["lang"] = "en_US";
    doc["params"]["kwargs"]["context"]["tz"] = "Europe/Brussels";
    doc["params"]["kwargs"]["context"]["uid"] = uid;
    doc["params"]["kwargs"]["context"]["allowed_company_ids"][0] = 1;
    doc["params"]["method"] = "method_direct_trigger";
    doc["params"]["model"] = "ir.cron";

    serializeJson(doc, payload);

    http.begin(endpoint.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Cookie", "session_id=" + session_id);

    int httpResponseCode = http.POST(payload);

    DynamicJsonDocument responseDoc(2048);

    if (httpResponseCode == 200) {
        deserializeJson(responseDoc, http.getString());
    } else {
        Serial.println("Failed to trigger method.");
    }

    http.end();
    return responseDoc;
}

DynamicJsonDocument post_payload(String  url, String session_id, int uid, const JsonDocument &payloadDoc) {
    HTTPClient http;
    Serial.println("In post Payload");

    // URL for the 'write' method on 'account.analytic.line'
     String endpoint = url + "/web/dataset/call_kw/account.analytic.line/write";
    // Serialize the provided JSON document as a string payload
    String payload;
    serializeJson(payloadDoc, payload);
    Serial.println(payload);

    // Begin the HTTP request
    http.begin(endpoint.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Cookie", "session_id=" + session_id);
    http.addHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/119.0");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Language", "en-US,en;q=0.5");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    http.addHeader("Referer", url + "/web");
    http.addHeader("Origin", url);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Sec-Fetch-Dest", "empty");
    http.addHeader("Sec-Fetch-Mode", "no-cors");
    http.addHeader("Sec-Fetch-Site", "same-origin");
    http.addHeader("TE", "trailers");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Cache-Control", "no-cache");

    // Make the HTTP POST request
    int httpResponseCode = http.POST(payload);
     
    // Initialize a JSON Document to store the response
    DynamicJsonDocument responseDoc(2048);
   
    // Check the response code
    if (httpResponseCode == 200) {
        // Parse the response payload into the JSON Document
        deserializeJson(responseDoc, http.getString());
        Serial.println("Success ");
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }

    // End the HTTP connection
    http.end();

    return responseDoc;
}

void UpdateTimeSheet(String session_id,int uid,int taskId,int projectId,float time_h, String dateString ){
     // Fill time sheet :
    Serial.println("Trying to time sheet");
    // Example JSON payload as per your provided example
    
 const size_t capacity = JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(2) +
                            JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(9) + 600;
    DynamicJsonDocument doc(capacity);

    // Construction de l'objet JSON
    doc["id"] = 260;
    doc["jsonrpc"] = "2.0";
    doc["method"] = "call";
    
    // Création du tableau 'args' dans 'params'
    JsonArray args = doc["params"].createNestedArray("args");
    doc["params"]["method"] = "write";
    doc["params"]["model"] = "project.task";
    
    // Ajout de l'ID de la tâche
    JsonArray taskIdArray = args.createNestedArray();
    taskIdArray.add(taskId); // ID de la tâche à mettre à jour

    // Construction de l'entrée 'timesheet_ids'
    JsonObject timesheet_ids_entry = args.createNestedObject();
    JsonArray timesheet_ids_array = timesheet_ids_entry.createNestedArray("timesheet_ids");
    JsonArray timesheet_entry = timesheet_ids_array.createNestedArray();
    timesheet_entry.add(0);
    timesheet_entry.add("virtual_10");

    // Ajout des détails du timesheet
    JsonObject timesheet_values = timesheet_entry.createNestedObject();
    timesheet_values["date"] = dateString;
    timesheet_values["user_id"] = 2;
    timesheet_values["employee_id"] = 1;
    timesheet_values["name"] = false;
    timesheet_values["unit_amount"] =time_h;
    timesheet_values["project_id"] = projectId;
    timesheet_values["task_id"] = taskId;

    // Contexte et autres paramètres
    JsonObject kwargs = doc["params"].createNestedObject("kwargs");
    JsonObject context = kwargs.createNestedObject("context");
    context["lang"] = "en_US";
    context["tz"] = "Europe/Brussels";
    context["uid"] = 2;
    JsonArray allowed_company_ids = context.createNestedArray("allowed_company_ids");
    allowed_company_ids.add(1);
    context["search_default_my_tasks"] = 1;
    context["search_default_open_tasks"] = 1;
    context["all_task"] = 0;
    JsonArray default_user_ids = context.createNestedArray("default_user_ids");
    JsonArray user_id_entry = default_user_ids.createNestedArray();
    user_id_entry.add(4);
    user_id_entry.add(2);


  // Serializez l'objet JSON pour voir la string qu'il génère
  String payload;
  Serial.println(payload);
  serializeJson(doc, payload);
  // Call the post_payload function with the necessary parameters
  DynamicJsonDocument response = post_payload(url,session_id, uid, doc);


  // Serialize the response to a string and print it
  String responseStr;
  serializeJson(response, responseStr);
  Serial.println("Response:");
  Serial.println(responseStr);
    
}

AuthResult authenticate_odoo(String url, String db, String login, String password) {
    HTTPClient http;
    AuthResult result;
    result.session_id = "";
    result.uid = -1;

    String auth_url = url + "/web/session/authenticate";
    String payload;

    // Construct the JSON payload.
    const size_t capacity = JSON_OBJECT_SIZE(3) + 
                            JSON_OBJECT_SIZE(3) + 
                            JSON_OBJECT_SIZE(3);
    DynamicJsonDocument doc(capacity);

    doc["jsonrpc"] = "2.0";
    doc["params"]["db"] = db;
    doc["params"]["login"] = login;
    doc["params"]["password"] = password;

    serializeJson(doc, payload);

    http.begin(auth_url.c_str());
    http.addHeader("Content-Type", "application/json");
    const char * headerkeys[] = {"User-Agent","Set-Cookie","Cookie","Date","Content-Type","Connection"} ;
    size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
    http.collectHeaders(headerkeys,headerkeyssize);
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode == 200) {
      String responseString = http.getString();
      Serial.println("Server Response: ");
      Serial.println(responseString);
      DynamicJsonDocument responseDoc(4096); 
      deserializeJson(responseDoc, responseString);
      
      // Directly navigate to the uid key and check its existence.
      int uid = responseDoc["result"]["uid"] | -1; 
      
      if (uid != -1) {
          result.session_id = extractSessionId(http.header("Set-Cookie"));;
          Serial.println(http.headers());
          result.uid = uid;
      } else {
          Serial.println("Authentication failed!");
      }
    }
    else {
      String responseString = http.getString();
      Serial.println("Server Response: ");
      Serial.println(responseString);}
    http.end();
    return result;
}

AuthResult authenticate_odoo(){
    return authenticate_odoo(url, db, login, password);
}

// Fonction pour envoyer une requête POST à Odoo et récupérer les ID et noms affichés
DynamicJsonDocument getProjectDataByTags(String session_id,String tags) {
    Serial.println(" getProjectData(String session_id)");


    // URL pour la requête
    String endpoint = url + "/web/dataset/call_kw/project.project/web_search_read";
    Serial.println(endpoint);

 
    String payload = "{\"id\":19,\"jsonrpc\":\"2.0\",\"method\":\"call\",\"params\":{\"model\":\"project.project\",\"method\":\"web_search_read\",\"args\":[],\"kwargs\":{\"limit\":80,\"offset\":0,\"order\":\"sequence ASC, name ASC, id ASC\",\"context\":{\"lang\":\"en_US\",\"tz\":\"Europe/Brussels\",\"uid\":2,\"allowed_company_ids\":[1],\"bin_size\":true,\"params\":{\"action\":224,\"model\":\"project.project\",\"view_type\":\"kanban\",\"cids\":1,\"menu_id\":144},\"default_allow_billable\":true},\"count_limit\":10001,\"domain\":[[\"tag_ids\",\"=\",\"";
    payload +=tags;
    payload += "\"]],\"fields\":[\"sequence\",\"message_needaction\",\"active\",\"is_favorite\",\"display_name\",\"partner_id\",\"privacy_visibility\",\"company_id\",\"date_start\",\"date\",\"allocated_hours\",\"user_id\",\"last_update_color\",\"tag_ids\",\"last_update_status\",\"stage_id\"]}}}";
    Serial.println(payload);
    
    // Configuration de la requête HTTP
    HTTPClient http;
    http.begin(endpoint.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Cookie", "session_id=" + session_id);
    http.addHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/119.0");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Language", "en-US,en;q=0.5");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    http.addHeader("Referer", url + "/web");
    http.addHeader("Origin", url);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Sec-Fetch-Dest", "empty");
    http.addHeader("Sec-Fetch-Mode", "no-cors");
    http.addHeader("Sec-Fetch-Site", "same-origin");
    http.addHeader("TE", "trailers");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Cache-Control", "no-cache");
    // Ajoutez les autres en-têtes comme nécessaire

    // Envoi de la requête POST
    int httpResponseCode = http.POST(payload);

    // Initialisation d'un document JSON pour la réponse
    DynamicJsonDocument responseDoc(16000);

    // Traitement de la réponse
    if (httpResponseCode == 200) {
        //Serial.println(http.getString());
        deserializeJson(responseDoc, http.getString());
        Serial.println("200 Code");
        
    } else {
        Serial.print("Erreur lors de l'envoi de la requête POST : ");
        Serial.println(httpResponseCode);
    }

    http.end();

    JsonArray records = responseDoc["result"]["records"];
    for (JsonObject record : records) {
        int id = record["id"];
        const char* displayName = record["display_name"];
        Serial.print("ID: ");
        Serial.print(id);
        Serial.print(", Nom affiché: ");
        Serial.println(displayName);
    }
    return responseDoc;
}

// Fonction pour envoyer une requête POST à Odoo et récupérer les ID et noms affichés
DynamicJsonDocument getProjectData(String session_id) {
    Serial.println(" getProjectData(String session_id)");
    HTTPClient http;

    // URL pour la requête
    String endpoint = url + "/web/dataset/call_kw/project.project/web_search_read";
    Serial.println(endpoint);

    // Sérialisation du JSON de la requête
    String payload;
    Serial.println(payload);
    payload = "{\"id\":19,\"jsonrpc\":\"2.0\",\"method\":\"call\",\"params\":{\"model\":\"project.project\",\"method\":\"web_search_read\",\"args\":[],\"kwargs\":{\"limit\":80,\"offset\":0,\"order\":\"sequence ASC, name ASC, id ASC\",\"context\":{\"lang\":\"en_US\",\"tz\":\"Europe/Brussels\",\"uid\":2,\"allowed_company_ids\":[1],\"bin_size\":true,\"params\":{\"action\":224,\"model\":\"project.project\",\"view_type\":\"kanban\",\"cids\":1,\"menu_id\":144},\"default_allow_billable\":true},\"count_limit\":10001,\"domain\":[],\"fields\":[\"sequence\",\"message_needaction\",\"active\",\"is_favorite\",\"display_name\",\"partner_id\",\"sale_line_id\",\"privacy_visibility\",\"company_id\",\"date_start\",\"date\",\"allocated_hours\",\"user_id\",\"last_update_color\",\"tag_ids\",\"last_update_status\",\"stage_id\"]}}}";
    Serial.println(payload);
    
    // Configuration de la requête HTTP
    http.begin(endpoint.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Cookie", "session_id=" + session_id);
    http.addHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/119.0");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Language", "en-US,en;q=0.5");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    http.addHeader("Referer", url + "/web");
    http.addHeader("Origin", url);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Sec-Fetch-Dest", "empty");
    http.addHeader("Sec-Fetch-Mode", "no-cors");
    http.addHeader("Sec-Fetch-Site", "same-origin");
    http.addHeader("TE", "trailers");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Cache-Control", "no-cache");
    // Ajoutez les autres en-têtes comme nécessaire

    // Envoi de la requête POST
    int httpResponseCode = http.POST(payload);

    // Initialisation d'un document JSON pour la réponse
    DynamicJsonDocument responseDoc(16000);

    // Traitement de la réponse
    if (httpResponseCode == 200) {
        //Serial.println(http.getString());
        deserializeJson(responseDoc, http.getString());
        Serial.println("200 Code");
        
    } else {
        Serial.print("Erreur lors de l'envoi de la requête POST : ");
        Serial.println(httpResponseCode);
    }

    http.end();

    JsonArray records = responseDoc["result"]["records"];
    for (JsonObject record : records) {
        int id = record["id"];
        const char* displayName = record["display_name"];
        Serial.print("ID: ");
        Serial.print(id);
        Serial.print(", Nom affiché: ");
        Serial.println(displayName);
    }
    return responseDoc;
}

float getRemainingsHours(String session_id,int projectId){
  String payload = "{\"id\":27,\"jsonrpc\":\"2.0\",\"method\":\"call\",\"params\":{\"model\":\"project.task\",\"method\":\"web_read_group\",\"args\":[],\"kwargs\":{\"orderby\":\"\",\"lazy\":true,\"expand_orderby\":null,\"expand_limit\":null,\"offset\":0,\"limit\":null,\"context\":{\"lang\":\"en_US\",\"tz\":\"Europe/Brussels\",\"uid\":2,\"allowed_company_ids\":[1],\"all_task\":0,\"default_user_ids\":[[4,2]]},\"groupby\":[\"project_id\"],\"domain\":[\"&\",\"&\",\"&\",[\"user_ids\",\"in\",2],\"&\",[\"is_blocked\",\"=\",false],[\"is_private\",\"=\",false],[\"is_closed\",\"=\",false],[\"project_id.id\",\"=\",";
  payload+=String(projectId);
  payload+="]],\"fields\":[\"activity_exception_decoration\",\"activity_exception_icon\",\"activity_state\",\"activity_summary\",\"activity_type_icon\",\"activity_type_id\",\"color\",\"priority\",\"stage_id\",\"user_ids\",\"partner_id\",\"sequence\",\"is_closed\",\"partner_is_company\",\"displayed_image_id\",\"active\",\"legend_blocked\",\"legend_normal\",\"legend_done\",\"activity_ids\",\"rating_count\",\"rating_avg\",\"allow_subtasks\",\"child_text\",\"is_private\",\"rating_active\",\"has_late_and_unreached_milestone\",\"allow_milestones\",\"progress\",\"remaining_hours\",\"planned_hours\",\"allow_timesheets\",\"encode_uom_in_days\",\"name\",\"project_id\",\"milestone_id\",\"commercial_partner_id\",\"email_from\",\"tag_ids\",\"date_deadline\",\"kanban_state\"]}}}";
  Serial.println(payload);
  // Configuration de la requête HTTP
  // URL pour la requête
    String endpoint = url + "/web/dataset/call_kw/project.task/web_read_group";
   HTTPClient http;
    http.begin(endpoint.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Cookie", "session_id=" + session_id);
    http.addHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/119.0");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Language", "en-US,en;q=0.5");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    http.addHeader("Referer", url + "/web");
    http.addHeader("Origin", url);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Sec-Fetch-Dest", "empty");
    http.addHeader("Sec-Fetch-Mode", "no-cors");
    http.addHeader("Sec-Fetch-Site", "same-origin");
    http.addHeader("TE", "trailers");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Cache-Control", "no-cache");
    // Ajoutez les autres en-têtes comme nécessaire

    // Envoi de la requête POST
    int httpResponseCode = http.POST(payload);

    // Initialisation d'un document JSON pour la réponse
    DynamicJsonDocument responseDoc(16000);

    // Traitement de la réponse
    if (httpResponseCode == 200) {
        //Serial.println(http.getString());
        deserializeJson(responseDoc, http.getString());
        // Supposons que 'planned_hours' est un float ou un int
        float plannedHours = responseDoc["result"]["groups"][0]["planned_hours"].as<float>();
        float remainingHours = responseDoc["result"]["groups"][0]["remaining_hours"].as<float>();
        
        // Maintenant, vous pouvez utiliser Serial.println sans ambiguïté
        Serial.println(plannedHours-remainingHours );
        http.end();
        return plannedHours-remainingHours ;
        
    } else {
        Serial.print("Erreur lors de l'envoi de la requête POST : ");
        Serial.println(httpResponseCode);
    }

    http.end();
  return 0.0;
  }

 // Fonction pour envoyer une requête POST à Odoo et récupérer les ID et noms affichés
DynamicJsonDocument getProjectTags(String session_id) {
  String payload = "{\"id\":39,\"jsonrpc\":\"2.0\",\"method\":\"call\",\"params\":{\"model\":\"project.tags\",\"method\":\"name_search\",\"args\":[],\"kwargs\":{\"name\":\"\",\"operator\":\"ilike\",\"args\":[],\"limit\":8,\"context\":{\"lang\":\"en_US\",\"tz\":\"Europe/Brussels\",\"uid\":2,\"allowed_company_ids\":[1]}}}}";
  // Configuration de la requête HTTP
  // URL pour la requête
    String endpoint = url + "/web/dataset/call_kw/project.task/web_read_group";
   HTTPClient http;
    http.begin(endpoint.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Cookie", "session_id=" + session_id);
    http.addHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/119.0");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Language", "en-US,en;q=0.5");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    http.addHeader("Referer", url + "/web");
    http.addHeader("Origin", url);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Sec-Fetch-Dest", "empty");
    http.addHeader("Sec-Fetch-Mode", "no-cors");
    http.addHeader("Sec-Fetch-Site", "same-origin");
    http.addHeader("TE", "trailers");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Cache-Control", "no-cache");
    // Ajoutez les autres en-têtes comme nécessaire

    // Envoi de la requête POST
    int httpResponseCode = http.POST(payload);

    // Initialisation d'un document JSON pour la réponse
    DynamicJsonDocument responseDoc(16000);

    // Traitement de la réponse
    if (httpResponseCode == 200) {
        //Serial.println(http.getString());
        deserializeJson(responseDoc, http.getString());
        http.end();
        return responseDoc ;
        
    } else {
        Serial.print("Erreur lors de l'envoi de la requête POST : ");
        Serial.println(httpResponseCode);
    }

    http.end();
  return responseDoc ;
  }

DynamicJsonDocument getTaskByProjectID(String session_id,int projectID) {
  String payload = "{\"id\":5,\"jsonrpc\":\"2.0\",\"method\":\"call\",\"params\":{\"model\":\"project.task\",\"method\":\"web_search_read\",\"args\":[],\"kwargs\":{\"limit\":80,\"offset\":0,\"order\":\"\",\"context\":{\"lang\":\"en_US\",\"tz\":\"Europe/Brussels\",\"uid\":2,\"allowed_company_ids\":[1],\"bin_size\":true,\"params\":{\"action\":215,\"active_id\":12,\"model\":\"project.task\",\"view_type\":\"list\",\"cids\":1,\"menu_id\":144},\"active_id\":12,\"active_ids\":[12],\"default_project_id\":12,\"show_project_update\":true},\"count_limit\":10001,\"domain\":[[\"display_project_id\",\"=\","+String(projectID)+"]],\"fields\":[\"activity_exception_decoration\",\"activity_exception_icon\",\"activity_state\",\"activity_summary\",\"activity_type_icon\",\"activity_type_id\",\"message_needaction\",\"is_closed\",\"sequence\",\"allow_milestones\",\"priority\",\"id\",\"child_text\",\"allow_subtasks\",\"name\",\"project_id\",\"milestone_id\",\"partner_id\",\"parent_id\",\"user_ids\",\"progress\",\"planned_hours\",\"effective_hours\",\"subtask_effective_hours\",\"total_hours_spent\",\"remaining_hours\",\"company_id\",\"activity_ids\",\"date_deadline\",\"tag_ids\",\"rating_active\",\"legend_normal\",\"legend_done\",\"legend_blocked\",\"kanban_state\",\"stage_id\",\"recurrence_id\"]}}}";// Configuration de la requête HTTP
   Serial.println(payload);
  // URL pour la requête
    String endpoint = url + "/web/dataset/call_kw/project.task/web_read_group";
   HTTPClient http;
    http.begin(endpoint.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Cookie", "session_id=" + session_id);
    http.addHeader("User-Agent", "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/119.0");
    http.addHeader("Accept", "*/*");
    http.addHeader("Accept-Language", "en-US,en;q=0.5");
    http.addHeader("Accept-Encoding", "gzip, deflate, br");
    http.addHeader("Referer", url + "/web");
    http.addHeader("Origin", url);
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Sec-Fetch-Dest", "empty");
    http.addHeader("Sec-Fetch-Mode", "no-cors");
    http.addHeader("Sec-Fetch-Site", "same-origin");
    http.addHeader("TE", "trailers");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Cache-Control", "no-cache");
    // Ajoutez les autres en-têtes comme nécessaire

    // Envoi de la requête POST
    int httpResponseCode = http.POST(payload);

    // Initialisation d'un document JSON pour la réponse
    DynamicJsonDocument responseDoc(16000);

    // Traitement de la réponse
    if (httpResponseCode == 200) {
        //Serial.println(http.getString());
        deserializeJson(responseDoc, http.getString());
        http.end();
        return responseDoc ;
        
    } else {
        Serial.print("Erreur lors de l'envoi de la requête POST : ");
        Serial.println(httpResponseCode);
    }

    http.end();
  return responseDoc ;
  }
