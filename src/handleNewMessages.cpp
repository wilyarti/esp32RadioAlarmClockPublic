//
// Created by wilyarti on 12/9/25.
//
#include "handleNewMessages.h"

// telegramChatID and bot are now in g_machine
void handleNewMessages(int numNewMessages) {
  // 1. Input Validation: Guard against invalid input parameters.
  if (numNewMessages <= 0) {
    Serial.println("[TG] handleNewMessages called with no new messages. Ignoring.");
    return;
  }

  // 2. Critical: Check if the bot object is valid.
  if (g_machine.bot == nullptr) {
    Serial.println("[TG] CRITICAL ERROR: Bot object is null! Cannot process messages.");
    return;
  }

  Serial.printf("[TG] Processing %d message(s).\n", numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {

    // 4. Check if the specific message structure is populated.
    // The library might have an array of messages, but not all elements are valid.
    if (g_machine.bot->messages[i].chat_id == 0) {
      Serial.printf("[TG] WARNING: Message %d has an invalid chat_id (0). Skipping.\n", i);
      continue; // Skip this message but process the next one
    }

    // 5. Check if the message text is present.
    String text = g_machine.bot->messages[i].text;
    if (text.isEmpty()) {
      Serial.printf("[TG] WARNING: Message %d from chat ID %lld has empty text. Skipping.\n", i, g_machine.bot->messages[i].chat_id);
      continue;
    }

    // 6. Store the chat ID and process the command
    g_machine.telegramChatID = String(g_machine.bot->messages[i].chat_id);
    Serial.printf("[TG] Received from %s: %s\n", g_machine.telegramChatID.c_str(), text.c_str());

    // 7. Safely remove the leading slash
    if (text.startsWith("/")) {
      text = text.substring(1);
    }

    // 8. Process the command with error handling
    try {
      processCommand(text, 1);
      Serial.println("[TG] Command processed successfully.");
    } catch (const std::exception &e) {
      Serial.printf("[TG] ERROR: Exception in processCommand for '%s': %s\n", text.c_str(), e.what());
      // Optionally, you can send an error message back to the user here.
      // g_machine.bot->sendMessage(g_machine.telegramChatID, "❌ An error occurred processing your command.", "");
    } catch (...) {
      Serial.printf("[TG] ERROR: Unknown exception in processCommand for '%s'\n", text.c_str());
    }
  }
}