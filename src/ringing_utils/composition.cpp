#include "ringing_utils.h"
#include <cstring>

namespace ringing_utils {

Composition Composition::PlainLead(Method method) {
    Composition to_return;
    to_return.number_of_changes = method.lead_length;
    to_return.stage = method.stage;
    std::vector<Change> raw_changes = method.getChanges();
    to_return.changes = std::vector<CompositionComponent>({});
    for (int i=0; i<to_return.number_of_changes; i++) {
        to_return.changes.push_back(CompositionComponent(raw_changes[i]));
    }
    return to_return;
}

Composition Composition::PlainCourse(Method method) {
    Composition to_return;
    to_return.stage = method.stage;
    to_return.changes = std::vector<CompositionComponent>({});
    // keep applying the changes for a lead until we get back to rounds
    int lead_length = method.lead_length;
    std::vector<Change> changes = method.getChanges();
    Row row = Row::Rounds(method.stage);
    Row rounds = Row::Rounds(method.stage);
    int counter = 0;
    do {
        row = row.ApplyChange(changes[counter%lead_length]);
        to_return.changes.push_back(CompositionComponent(changes[counter%lead_length]));
        counter++;
    } while (strcmp(row.ToString().c_str(), rounds.ToString().c_str()) != 0 && counter<600);

    to_return.number_of_changes = counter;
    return to_return;
}

}