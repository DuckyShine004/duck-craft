#pragma once

namespace manager {

class Manager {
  public:
    Manager(const Manager &) = delete;

    Manager &operator=(const Manager &) = delete;

    virtual void initialise();

  protected:
    Manager();

    ~Manager();
};

} // namespace manager
